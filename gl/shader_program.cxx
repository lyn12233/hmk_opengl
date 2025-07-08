#define _CRT_SECURE_NO_WARNINGS

#include "shader_program.hxx"
#include "checkfail.hxx"
#include "shader.hxx"

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include <spdlog/spdlog.h>

using glwrapper::ShaderProgram;
using std::filesystem::path;

ShaderProgram::ShaderProgram(
    std::string vshader_path, std::string fshader_path, std::string vshader_str,
    std::string fshader_str, std::string gshader_path, std::string gshader_str
) :
    vshader(vshader_path, GL_VERTEX_SHADER, vshader_str),
    fshader(fshader_path, GL_FRAGMENT_SHADER, fshader_str),
    gshader(gshader_path, GL_GEOMETRY_SHADER, gshader_str) {

    init();
}

ShaderProgram::ShaderProgram(
    std::string vshader_str, std::string fshader_str, std::string gshader_str
) {
    if (vshader_str.find('#') != std::string::npos) {
        vshader = Shader("", GL_VERTEX_SHADER, vshader_str);
    } else {
        vshader = Shader(find_path(vshader_str).string(), GL_VERTEX_SHADER, "");
    }
    if (fshader_str.find('#') != std::string::npos) {
        fshader = Shader("", GL_FRAGMENT_SHADER, fshader_str);
    } else {
        fshader = Shader(find_path(fshader_str).string(), GL_FRAGMENT_SHADER, "");
    }
    if (gshader_str.find('#') != std::string::npos) {
        gshader = Shader("", GL_GEOMETRY_SHADER, gshader_str);
    } else {
        gshader = Shader(find_path(gshader_str).string(), GL_GEOMETRY_SHADER, "");
    }
    init();
}

void ShaderProgram::init() {

    if (!vshader.exist() || !fshader.exist()) {
        spdlog::error("vshader and fshader is required {},{}", vshader.exist(), fshader.exist());
        exit(-1);
    }

    int success;

    ID_ = glCreateProgram();
    vshader.attach_to_program(ID_);
    fshader.attach_to_program(ID_);
    if (gshader.exist()) gshader.attach_to_program(ID_);

    spdlog::info("linking shader program...\n");

    glLinkProgram(ID_);

    glGetProgramiv(ID_, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetProgramInfoLog(ID_, 512, NULL, info_log);
        spdlog::error("shader program link failed: {}", info_log);
        exit(-1);
    }
    spdlog::info("shader program linked successfully: {}", ID_);
}

ShaderProgram::ShaderProgram(ShaderProgram &&o) :
    ID_(o.ID_), vshader(std::move(o.vshader)), fshader(std::move(o.fshader)),
    gshader(std::move(o.gshader)) {

    o.ID_ = 0;
}

ShaderProgram::~ShaderProgram() {
    if (ID_ == 0) return;
    glDeleteProgram(ID_);
}
void ShaderProgram::use() {
    MY_CHECK_FAIL;
    glUseProgram(ID_);
    // MY_CHECK_FAIL;//failed
}

path ShaderProgram::find_path(path p) {
    namespace fs = std::filesystem;

    if (p.empty()) return p;
    if (fs::exists(p) && fs::is_regular_file(p)) return p;
    if (p.is_absolute()) return p;

    const char *env_path = std::getenv("PATH");
    if (!env_path) return p;
    std::string paths(env_path);

#ifdef _WIN32
    constexpr char sep = ';';
#else
    constexpr char sep = ':';
#endif

    size_t start = 0, end = 0;
    while (true) {
        end = paths.find(sep, start);

        path dir       = paths.substr(start, end - start);
        path candidate = dir / p;
        if (fs::exists(candidate) && fs::is_regular_file(candidate)) {
            return candidate;
        }

        if (end == std::string::npos) break;
        start = end + 1;
    }

    return p;
}