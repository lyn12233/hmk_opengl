#include "glfw_inst.hxx"

#include <sstream>
#include <string>

#include <spdlog/spdlog.h>

using glwrapper::GlfwInst;

GlfwInst::GlfwInst(std::string version, int profile, bool do_load) {
    glfwInit();
    std::string version_major, version_minor;
    std::getline(std::istringstream(version), version_major, '.');
    std::getline(std::istringstream(version), version_minor, '.');

    spdlog::info("opengl version: {}.{}", version_major, version_minor);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, atoi(version_major.c_str()));
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, atoi(version_minor.c_str()));
    glfwWindowHint(GLFW_OPENGL_PROFILE, profile);

    // load_proc
    if (do_load) load_proc();
}

GlfwInst::~GlfwInst() {
    spdlog::info("glfw terminate");
    glfwTerminate();
}

void GlfwInst::load_proc() {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        spdlog::error("failed GLAD loading proc");
        exit(-1);
    }
}