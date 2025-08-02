#pragma once

#include "config.hxx"

#include <string>

#ifndef __gl_h_
    #include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>

namespace glwrapper {

    class GlfwInst {
        public:
        GlfwInst(                                     //
            std::string version = DEFAULT_GL_VERSION, //
            int         profile = GLFW_OPENGL_CORE_PROFILE
        );
        GlfwInst(GlfwInst &&)      = delete;
        GlfwInst(const GlfwInst &) = delete;
        ~GlfwInst();

        void load_proc();
    };
} // namespace glwrapper