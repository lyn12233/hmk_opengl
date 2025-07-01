#pragma once

#include <string>

#ifndef __gl_h_
    #include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>

class GlfwInst {
    public:
    GlfwInst(
        std::string version = "3.3", int profile = GLFW_OPENGL_CORE_PROFILE, bool do_load = false
    );
    GlfwInst(GlfwInst &&)      = delete;
    GlfwInst(const GlfwInst &) = delete;
    ~GlfwInst();

    void load_proc();
};