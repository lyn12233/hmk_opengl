#include "shader_program.hxx"

#include "checkfail.hxx"

#include <spdlog/spdlog.h>

ShaderProgram::ShaderProgram(
    std::string vshader_path, std::string fshader_path, std::string vshader_str,
    std::string fshader_str, std::string gshader_path, std::string gshader_str
)
    : vshader(vshader_path, GL_VERTEX_SHADER, vshader_str),
      fshader(fshader_path, GL_FRAGMENT_SHADER, fshader_str),
      gshader(gshader_path, GL_GEOMETRY_SHADER, gshader_str) {

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

ShaderProgram::ShaderProgram(ShaderProgram &&o)
    : ID_(o.ID_), vshader(std::move(o.vshader)), fshader(std::move(o.fshader)),
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