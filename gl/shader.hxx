#pragma once

#include <iostream>
#include <string>

#include <spdlog/spdlog.h>

#ifndef __gl_h_
    #include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>

class Shader {
    public:
    Shader(std::string file_path, GLenum shader_type, std::string src = "");
    Shader(Shader &&o);
    Shader(const Shader &) = delete;
    ~Shader();

    void set_value(std::string name, float value);
    void set_value(std::string name, int value);
    void attach_to_program(GLuint prog_id);

    // readonly's
    inline auto ID() { return ID_; }
    inline bool exist() { return ID_ != 0; }

    protected:
    GLuint      ID_;
    std::string shader_name;
};