#include "Shader.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "arena_alloc.h"
#include "build_config.h"

// -----------------------------
// 🔍 Debug OpenGL Error Checker
// -----------------------------
static void glCheckError(const char* context) {
#ifdef DEBUG
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        const char* errStr = "UNKNOWN";
        switch (err) {
            case GL_INVALID_ENUM:      errStr = "GL_INVALID_ENUM"; break;
            case GL_INVALID_VALUE:     errStr = "GL_INVALID_VALUE"; break;
            case GL_INVALID_OPERATION: errStr = "GL_INVALID_OPERATION"; break;
            case GL_OUT_OF_MEMORY:     errStr = "GL_OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: errStr = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        fprintf(stderr, "[OpenGL Error] %s: %s (0x%x)\n", context, errStr, err);
    }
#endif
}


// -----------------------------
// 📄 Helper: Read Shader File
// -----------------------------
static std::string readFile(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Failed to open file: " + path.string());

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}


// -----------------------------
// ⚙️ Helper: Compile Shader Stage
// -----------------------------
static GLuint compileStage(GLenum type, const char* src, const char* name) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        fprintf(stderr, "❌ [%s] Shader compilation failed:\n%s\n", name, infoLog);
        throw std::runtime_error("Shader compilation failed");
    }

    return shader;
}


// -----------------------------
// 🔧 Constructor
// -----------------------------
Shader::Shader(const std::vector<std::pair<const char*, GLenum>>& sources, const char* name)
    : ID(0), shader_name(name)
{
    printf("[Shader] Creating program: %s\n", name);
    ID = glCreateProgram();
    if (!ID) {
        fprintf(stderr, "❌ Failed to create GL program for '%s'\n", name);
        return;
    }

    std::vector<GLuint> compiledShaders;
    compiledShaders.reserve(sources.size());

    try {
        for (auto& [src, type] : sources) {
            GLuint shader = compileStage(type, src, name);
            glAttachShader(ID, shader);
            compiledShaders.push_back(shader);
        }

        glLinkProgram(ID);
        GLint success;
        glGetProgramiv(ID, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[2048];
            glGetProgramInfoLog(ID, sizeof(infoLog), nullptr, infoLog);
            fprintf(stderr, "❌ Program linking failed for '%s':\n%s\n", name, infoLog);
            glDeleteProgram(ID);
            ID = 0;
            throw std::runtime_error("Shader link error");
        }

    } catch (...) {
        for (auto shader : compiledShaders)
            glDeleteShader(shader);
        throw;
    }

    for (auto shader : compiledShaders)
        glDeleteShader(shader);

    printf("Loaded shader '%s' (Program ID %u)\n", name, ID);
    glCheckError(("Shader::Shader[" + std::string(name) + "]").c_str());
}


// Old-style constructor for compatibility
Shader::Shader(const char* vertexSrc, const char* fragmentSrc, const char* name)
    : Shader({{vertexSrc, GL_VERTEX_SHADER}, {fragmentSrc, GL_FRAGMENT_SHADER}}, name) {}


// -----------------------------
// 🧹 Destructor
// -----------------------------
Shader::~Shader() {
    if (glIsProgram(ID)) {
        printf("[Shader] Destroying program ID %u\n", ID);
        glDeleteProgram(ID);
    }
}


// -----------------------------
// ⚙️ Shader::reload()
// -----------------------------
void Shader::reload() {
    std::filesystem::path shaders_dir = build_config::shaders_dir;

    auto fragPath = shaders_dir / (shader_name + ".frag");
    auto vertPath = shaders_dir / (shader_name + ".vert");

    printf("[Shader] Reloading '%s'\n", shader_name.c_str());
    try {
        std::string fragSource = readFile(fragPath);
        std::string vertSource = readFile(vertPath);

        GLuint newProgram = glCreateProgram();
        GLuint vertShader = compileStage(GL_VERTEX_SHADER, vertSource.c_str(), shader_name.c_str());
        GLuint fragShader = compileStage(GL_FRAGMENT_SHADER, fragSource.c_str(), shader_name.c_str());

        glAttachShader(newProgram, vertShader);
        glAttachShader(newProgram, fragShader);
        glLinkProgram(newProgram);

        GLint success;
        glGetProgramiv(newProgram, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[2048];
            glGetProgramInfoLog(newProgram, sizeof(infoLog), nullptr, infoLog);
            fprintf(stderr, "❌ Reload link failed for '%s':\n%s\n", shader_name.c_str(), infoLog);
            glDeleteProgram(newProgram);
            throw std::runtime_error("Shader reload link error");
        }

        // Success — replace old program
        if (glIsProgram(ID))
            glDeleteProgram(ID);
        ID = newProgram;

        glDeleteShader(vertShader);
        glDeleteShader(fragShader);

        printf("🔁 Reloaded shader '%s' (Program ID %u)\n", shader_name.c_str(), ID);
        glCheckError(("Shader::reload[" + shader_name + "]").c_str());

    } catch (const std::exception& e) {
        fprintf(stderr, "❌ Shader reload failed for '%s': %s\n", shader_name.c_str(), e.what());
    }
}


// -----------------------------
// 🧠 loadFromName()
// -----------------------------
Shader* Shader::loadFromName(const std::string name, arena::Allocator<std::byte>& allocator) {
    std::filesystem::path shaders_dir = build_config::shaders_dir;

    auto fragPath = shaders_dir / (name + ".frag");
    auto vertPath = shaders_dir / (name + ".vert");

    printf("[Shader] Loading from name '%s'\n", name.c_str());
    printf("  Vertex:   %s\n", vertPath.string().c_str());
    printf("  Fragment: %s\n", fragPath.string().c_str());

    try {
        std::string fragSource = readFile(fragPath);
        std::string vertSource = readFile(vertPath);

        void* pShader = allocator.allocate(sizeof(Shader), nullptr);
        if (!pShader) {
            fprintf(stderr, "❌ Arena allocation failed for shader '%s'\n", name.c_str());
            return nullptr;
        }

        Shader* shader = new(pShader) Shader({
            { vertSource.c_str(), GL_VERTEX_SHADER },
            { fragSource.c_str(), GL_FRAGMENT_SHADER }
        }, name.c_str());

        glCheckError(("Shader::loadFromName[" + name + "]").c_str());
        return shader;

    } catch (const std::exception& e) {
        fprintf(stderr, "❌ Failed to load shader '%s': %s\n", name.c_str(), e.what());
        return nullptr;
    }
}


// -----------------------------
// 🎨 Uniforms + Utilities
// -----------------------------
void Shader::use() const { glUseProgram(ID); }
bool Shader::isValid() const { return ID && glIsProgram(ID); }

void Shader::uniformNotFound(const std::string& name) const {
    fprintf(stderr, "⚠️  (%s) Missing uniform '%s' (program %u)\n",
        shader_name.c_str(), name.c_str(), ID);
}

void Shader::setFloat(const std::string &name, float value) const {
    GLint loc = glGetUniformLocation(ID, name.c_str());
    if (loc == -1) uniformNotFound(name); else glUniform1f(loc, value);
}

void Shader::setInt(const std::string &name, int value) const {
    GLint loc = glGetUniformLocation(ID, name.c_str());
    if (loc == -1) uniformNotFound(name); else glUniform1i(loc, value);
}

void Shader::setVec2(const std::string &name, const glm::vec2 &v) const {
    GLint loc = glGetUniformLocation(ID, name.c_str());
    if (loc == -1) uniformNotFound(name); else glUniform2fv(loc, 1, glm::value_ptr(v));
}

void Shader::setVec3(const std::string &name, const glm::vec3 &v) const {
    GLint loc = glGetUniformLocation(ID, name.c_str());
    if (loc == -1) uniformNotFound(name); else glUniform3fv(loc, 1, glm::value_ptr(v));
}

void Shader::setVec4(const std::string &name, const glm::vec4 &v) const {
    GLint loc = glGetUniformLocation(ID, name.c_str());
    if (loc == -1) uniformNotFound(name); else glUniform4fv(loc, 1, glm::value_ptr(v));
}

void Shader::setMat4(const std::string &name, const glm::mat4 &m) const {
    GLint loc = glGetUniformLocation(ID, name.c_str());
    if (loc == -1) uniformNotFound(name); else glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m));
}

void Shader::useCamera(Camera &camera) const {
    setMat4("view", camera.getViewMatrix());
    setMat4("projection", camera.getProjectionMatrix());
}

void Shader::useCameraWorldMesh(Camera &camera) {
    setFloat("ambientStrength", camera.lighting_shader_config.ambientStrength);
    setFloat("diffuseStrength", camera.lighting_shader_config.diffuseStrength);
    setFloat("specularStrength", camera.lighting_shader_config.specularStrength);
    setFloat("shininess", camera.lighting_shader_config.shininess);
    setFloat("vibrancy", camera.lighting_shader_config.vibrancy);
    setFloat("contrast", camera.lighting_shader_config.contrast);
    setFloat("ambientOcclusion", camera.lighting_shader_config.ambientOcclusion);
    setVec3("fogColor", camera.lighting_shader_config.fogColor);
    setFloat("fogStart", camera.lighting_shader_config.fogStart);
    setFloat("fogEnd", camera.lighting_shader_config.fogEnd);
    setInt("pcfRadius", camera.lighting_shader_config.pcfRadius);
    setInt("noPostProcessing", camera.lighting_shader_config.bypassPostProcessing);
}

void Shader::useCameraLighting(Camera &camera) {
    setVec3("lightColor", camera.lighting_shader_config.lightColor);
    setVec3("viewPos", camera.position);
}
