
#define STB_IMAGE_IMPLEMENTATION

#include "Mesh.h"
#include "Camera.h"


#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "Vertex.h"

Mesh::Mesh(Mesh&& o) noexcept {
    vao = o.vao; vbo = o.vbo; ebo = o.ebo; indexCount = o.indexCount;
    position = o.position; rotation = o.rotation; scale = o.scale;
    o.vao = o.vbo = o.ebo = 0; o.indexCount = 0;
}
Mesh& Mesh::operator=(Mesh&& o) noexcept {
    if (this != &o) {
        if (vao) glDeleteVertexArrays(1, &vao);
        if (vbo) glDeleteBuffers(1, &vbo);
        if (ebo) glDeleteBuffers(1, &ebo);
        vao = o.vao; vbo = o.vbo; ebo = o.ebo; indexCount = o.indexCount;
        position = o.position; rotation = o.rotation; scale = o.scale;
        o.vao = o.vbo = o.ebo = 0; o.indexCount = 0;
    }
    return *this;
}

void Mesh::useTransformation(glm::vec3 offset) {
    this->offset = offset;
}

void Mesh::draw() const {
    

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}



Mesh::Mesh(
  const std::vector<Vertex>& vertices,
  const std::vector<unsigned int>& indices)
{
    // Generate IDs
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    // Bind VAO first
    glBindVertexArray(vao);

    // VBO
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 vertices.size() * sizeof(Vertex),
                 vertices.data(),
                 GL_STATIC_DRAW);

    // EBO (bound while VAO is active → VAO remembers it)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indices.size() * sizeof(unsigned int),
                 indices.data(),
                 GL_STATIC_DRAW);




    // positions (loc 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // normals (loc 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3*sizeof(float)));

    // texcoords (loc 2)  ← THIS must exist & match your vertex shader
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));



    // stride = sizeof(Vertex)
    // glEnableVertexAttribArray(0); // position
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    //
    // glEnableVertexAttribArray(1); // normal
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    //
    // glEnableVertexAttribArray(2); // color
    // glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    //
    // glEnableVertexAttribArray(3); // texcoord
    // glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    //
    // glEnableVertexAttribArray(4); // bary
    // glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bary));






    // Unbind VAO (EBO binding is preserved inside it!)
    glBindVertexArray(0);

    // Store index count for draw()
    indexCount = static_cast<GLsizei>(indices.size());
}

// void Mesh::draw() const {
//     glBindVertexArray(vao);
//     glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, 0);
//     glBindVertexArray(0);
// }

glm::mat4 Mesh::getModelMatrix() const {
  glm::mat4 model(1.0f);

  model = glm::translate(model, position);
  model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1, 0, 0));
  model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0, 1, 0));
  model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0, 0, 1));
  model = glm::scale(model, scale);

  return model;
}

std::optional<Mesh> Mesh::loadOBJ(const std::string& path) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
        std::cerr << "OBJ load failed: " << err << std::endl;
        return std::nullopt;
    }

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // deduplication map
    struct Key {
        int v, vt, vn;
        bool operator==(const Key& o) const { return v == o.v && vt == o.vt && vn == o.vn; }
    };

    struct KeyHash {
        size_t operator()(const Key& k) const {
            size_t h1 = std::hash<int>()(k.v);
            size_t h2 = std::hash<int>()(k.vt);
            size_t h3 = std::hash<int>()(k.vn);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };

    std::unordered_map<Key, unsigned int, KeyHash> uniqueVerts;

    for (const auto& shape : shapes) {
        for (const auto& idx : shape.mesh.indices) {
            Key key{ idx.vertex_index, idx.texcoord_index, idx.normal_index };

            if (uniqueVerts.count(key) == 0) {
                Vertex vert{};

                // --- position ---
                if (idx.vertex_index >= 0) {
                    vert.position = {
                        attrib.vertices[3 * idx.vertex_index + 0],
                        attrib.vertices[3 * idx.vertex_index + 1],
                        attrib.vertices[3 * idx.vertex_index + 2]
                    };
                } else {
                    vert.position = {0.0f, 0.0f, 0.0f};
                }

                // --- normal ---
                if (idx.normal_index >= 0 && !attrib.normals.empty()) {
                    vert.normal = {
                        attrib.normals[3 * idx.normal_index + 0],
                        attrib.normals[3 * idx.normal_index + 1],
                        attrib.normals[3 * idx.normal_index + 2]
                    };
                } else {
                    vert.normal = {0.0f, 0.0f, 1.0f}; // fallback
                }

                // --- texcoord ---
                if (idx.texcoord_index >= 0 && !attrib.texcoords.empty()) {
                    vert.texCoord = {
                        attrib.texcoords[2 * idx.texcoord_index + 0],
                        attrib.texcoords[2 * idx.texcoord_index + 1]
                    };
                } else {
                    vert.texCoord = {0.0f, 0.0f}; // fallback
                }

                // --- color ---
                vert.color = {1.0f, 1.0f, 1.0f}; // default white

                unsigned int newIndex = static_cast<unsigned int>(vertices.size());
                vertices.push_back(vert);
                uniqueVerts[key] = newIndex;
            }

            indices.push_back(uniqueVerts[key]);
        }
    }



    // return Mesh(vertices, indices);
    return std::optional<Mesh>(std::in_place, vertices, indices);
}



void Mesh::setPosition(const glm::vec3 &pos) { position = pos; }
void Mesh::setRotation(const glm::vec3 &rot) { rotation = rot; }
void Mesh::setScale(const glm::vec3 &s) { scale = s; }

Mesh::~Mesh() {
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &ebo);
  glDeleteVertexArrays(1, &vao);
}
