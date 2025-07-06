#pragma once

#include "shader.hxx"

#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>

#ifndef __gl_h_
    #include <glad/glad.h>
#endif

using glm::mat3;
using glm::mat4;
using glm::value_ptr;
using glm::vec3;
using glm::vec4;
using std::is_same_v;

class ShaderProgram {
    public:
    ShaderProgram(
        std::string vshader_path, std::string fshader_path, std::string vshader_str,
        std::string fshader_str, std::string gshader_path, std::string gshader_str
    );
    ShaderProgram(std::string vshader = "", std::string fshader = "", std::string gshader = "");
    void init();
    ShaderProgram(ShaderProgram &&o);
    ShaderProgram(const ShaderProgram &) = delete;
    ~ShaderProgram();

    void use();

    template<typename T> void set_value(const char *name, T value);

    // readonly's
    inline auto ID() { return ID_; };

    protected:
    GLuint                       ID_;
    Shader                       vshader;
    Shader                       fshader;
    Shader                       gshader;
    static std::filesystem::path find_path(std::filesystem::path p);
};

template<typename T> void ShaderProgram::set_value(const char *name, T value) {
    // use program
    use();
    // get parm location and checkfail
    auto location = glGetUniformLocation(ID_, name);
    if (location == -1) {
        spdlog::error("uniform parm not found: {} (prog_id: {})", name, ID_);
        exit(-1);
    }

    // set parm according to type
    static_assert(
        is_same_v<T, int> || is_same_v<T, unsigned int> || is_same_v<T, float> ||
            is_same_v<T, vec3> || is_same_v<T, mat3> || is_same_v<T, vec4> || is_same_v<T, mat4> ||
            is_same_v<T, std::vector<vec4>>,
        "T not supported"
    );

    if constexpr (is_same_v<T, float>) {
        glUniform1f(location, value);
    } else if constexpr (is_same_v<T, int> || is_same_v<T, unsigned int>) {
        glUniform1i(location, value);
    }
    // vec3, mat3
    else if constexpr (is_same_v<T, vec3>) {
        glUniform3fv(location, 1, value_ptr(value));
    } else if constexpr (is_same_v<T, mat3>) {
        glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(value));
    }
    // vec4, mat4
    else if constexpr (is_same_v<T, vec4>) {
        glUniform4fv(location, 1, value_ptr(value));
    } else if constexpr (is_same_v<T, mat4>) {
        glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(value));
    } else if constexpr (is_same_v<T, std::vector<vec4>>) {
        glUniform4fv(location, value.size(), value_ptr(value[0]));
    }
}