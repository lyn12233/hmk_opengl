#include "shader.hxx"

#include <cstddef>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>

#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

using glwrapper::Shader;

Shader::Shader(std::string file_path, GLenum shader_type, std::string src) :
    shader_name(file_path) {

    if (!glClear) {
        spdlog::error("Shader::Shader: glad uninitialized\n");
        exit(-1);
    }
    if (file_path == "" && src == "") {
        ID_ = 0;
        return;
    }

    // get shader source
    std::string code;
    if (src == "") { // shader source not exist
        std::ifstream     file;
        std::stringstream stream;
        file.open(file_path); // mode read
        if (!file.is_open()) {
            spdlog::error("can't open file: {}", file_path);
            exit(-1);
        }
        stream << file.rdbuf();
        file.close();
        code = stream.str();
    } else { // shader source exist
        code = src;
    } // get shader source/>

    spdlog::info("compiling: \n{}", code);

    const char *c_code = code.c_str();
    {
        int  success;
        char info_log[200];

        ID_ = glCreateShader(shader_type);
        glShaderSource(ID(), 1, &c_code, NULL);
        glCompileShader(ID());
        // check compile errors
        glGetShaderiv(ID(), GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(ID(), 200, NULL, info_log);
            spdlog::error("shader compile error:{}", info_log);
            exit(-1);
        }
        spdlog::info("successfully compiled");
    }
}

Shader::Shader(Shader &&o) : ID_(o.ID_), shader_name(o.shader_name) { o.ID_ = 0; }

void Shader::operator=(Shader &&o) {
    if (this != &o) {
        cleanup();
        ID_   = o.ID_;
        o.ID_ = 0;
    }
}

void Shader::cleanup() {
    if (ID_ == 0) return;
    spdlog::debug("Shader::cleanup: {}", shader_name);
    glDeleteShader(ID_);
}
void Shader::attach_to_program(GLuint prog_id) {
    spdlog::debug("attaching shader: {}", shader_name);

    int nb_shaders1, nb_shaders2;
    glGetProgramiv(prog_id, GL_ATTACHED_SHADERS, &nb_shaders1);
    glAttachShader(prog_id, ID());
    glGetProgramiv(prog_id, GL_ATTACHED_SHADERS, &nb_shaders2);

    if (nb_shaders1 == nb_shaders2) {
        spdlog::error("shader not attached correctly: {}->{}", nb_shaders1, nb_shaders2);
        exit(-1);
    }
}
