#pragma once

#include "shader.hxx"

#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>

#ifndef __gl_h_
    #include <glad/glad.h>
#endif

using glm::mat4;
using glm::vec4;

class ShaderProgram {
    public:
    ShaderProgram(
        std::string vshader_path, std::string fshader_path, std::string vshader_str = "",
        std::string fshader_str = "", std::string gshader_path = "", std::string gshader_str = ""
    );
    ShaderProgram(ShaderProgram &&o);
    ShaderProgram(const ShaderProgram &) = delete;
    ~ShaderProgram();

    void use();

    template<typename T> void set_value(const char *name, T value);

    // readonly's
    inline auto ID() { return ID_; };

    protected:
    GLuint ID_;
    Shader vshader;
    Shader fshader;
    Shader gshader;
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
        std::is_same_v<T, int> || std::is_same_v<T, unsigned int> || std::is_same_v<T, float> ||
            std::is_same_v<T, glm::vec3> || std::is_same_v<T, mat4> ||
            std::is_same_v<T, std::vector<vec4>>,
        "T not supported"
    );

    if constexpr (std::is_same_v<T, float>) {
        glUniform1f(location, value);
    } else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, unsigned int>) {
        glUniform1i(location, value);
    } else if constexpr (std::is_same_v<T, glm::vec3>) {
        glUniform3fv(location, 1, &value[0]);
    } else if constexpr (std::is_same_v<T, mat4>) {
        glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
    } else if constexpr (std::is_same_v<T, std::vector<vec4>>) {
        glUniform4fv(location, value.size(), glm::value_ptr(value[0]));
    }
}