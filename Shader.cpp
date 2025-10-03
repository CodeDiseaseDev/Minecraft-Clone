#include "Shader.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>

Shader::Shader(const std::vector<std::pair<const char*, GLenum>>& sources, const char* name) {
    ID = glCreateProgram();
    std::vector<unsigned int> compiledShaders;

    for (auto& [src, type] : sources) {
        unsigned int shader = compile(type, src);
        glAttachShader(ID, shader);
        compiledShaders.push_back(shader);
    }

    glLinkProgram(ID);

    int success;
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(ID, 512, nullptr, infoLog);
        throw std::runtime_error(std::string("Program linking failed: ") + infoLog);
    }

    for (auto shader : compiledShaders) {
        glDeleteShader(shader);
    }

    printf("Loaded '%s' shader.\n", name);
}

// Old style constructor — just forwards to new one
Shader::Shader(const char* vertexSrc, const char* fragmentSrc, const char* name)
    : Shader({{vertexSrc, GL_VERTEX_SHADER}, {fragmentSrc, GL_FRAGMENT_SHADER}}, name) {}

void Shader::use() const {
    glUseProgram(ID);
}

void Shader::setFloat(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setVec3(const std::string &name, const glm::vec3 &v) const {
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(v));
}

void Shader::setVec2(const std::string &name, const glm::vec2 &v) const {
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(v)); // fixed bug (was 3fv)
}

void Shader::setVec4(const std::string &name, const glm::vec4 &v) const {
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(v));
}

void Shader::setMat4(const std::string &name, const glm::mat4& mat) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()),
                       1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setInt(const std::string &name, const int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::useCamera(Camera &camera) const {
    setMat4("view",         camera.getViewMatrix());
    setMat4("projection",   camera.getProjectionMatrix());
    // setVec3("sunDir",       camera.lighting_shader_config.sunDir);
    setVec3("lightColor",   camera.lighting_shader_config.lightColor);
    setVec3("viewPos",      camera.position);

    setFloat("ambientStrength",    camera.lighting_shader_config.ambientStrength);
    setFloat("diffuseStrength",    camera.lighting_shader_config.diffuseStrength);
    setFloat("specularStrength",   camera.lighting_shader_config.specularStrength);
    setFloat("shininess",          camera.lighting_shader_config.shininess);
    setFloat("vibrancy",           camera.lighting_shader_config.vibrancy);
    setFloat("contrast",           camera.lighting_shader_config.contrast);

    setVec3("fogColor", camera.lighting_shader_config.fogColor);
    setFloat("fogStart", camera.lighting_shader_config.fogStart);
    setFloat("fogEnd", camera.lighting_shader_config.fogEnd);
    setFloat("fogDensity", camera.lighting_shader_config.fogDensity);

    setInt("pcfRadius", camera.lighting_shader_config.pcfRadius);
}

std::shared_ptr<Shader> Shader::loadFromName(const std::string name) {
    std::ifstream frag, vert;

    std::filesystem::path shaders_dir = std::filesystem::current_path() / "shaders";

    frag.open(shaders_dir / (name + ".frag"));
    vert.open(shaders_dir / (name + ".vert"));

    if (!frag || !vert) {
        throw std::runtime_error("Failed to load shaders");
    }

    std::stringstream fragBuffer, vertBuffer;
    fragBuffer << frag.rdbuf();
    vertBuffer << vert.rdbuf();

    std::string fragSource = fragBuffer.str();
    std::string vertSource = vertBuffer.str();

    frag.close();
    vert.close();

    return std::make_shared<Shader>(
        vertSource.c_str(), fragSource.c_str(),
        name.c_str()
    );
}

unsigned int Shader::compile(unsigned int type, const char *src) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        throw std::runtime_error(std::string("Shader compilation failed: ") + infoLog);
    }

    return shader;
}
