#include "drawable_frame.hxx"
#include "buffer_objects.hxx"
#include "shader_program.hxx"
#include "texture_objects.hxx"
#include <spdlog/spdlog.h>

using glwrapper::BufferObject;
using glwrapper::ShaderProgram;
using glwrapper::TextureObject;
using glwrapper::VertexArrayObject;
using glwrapper::VertexBufferObject;
using mf::DrawableFrame;

// statics
std::optional<VertexArrayObject>  DrawableFrame::cp_vao{};
std::optional<VertexBufferObject> DrawableFrame::cp_vbo{};
std::optional<BufferObject>       DrawableFrame::cp_ebo{};
int                               DrawableFrame::nb_inst_ = 0;

float DrawableFrame::cp_vertices[16] = {
    // pos//tex_uv
    -1.0f, 1.0f,  0.0f, 1.0f, // Top-left
    -1.0f, -1.0f, 0.0f, 0.0f, // Bottom-left
    1.0f,  -1.0f, 1.0f, 0.0f, // Bottom-right
    1.0f,  1.0f,  1.0f, 1.0f  // Top-right
};
GLuint                         DrawableFrame::cp_indices[6] = {0, 1, 2, 0, 2, 3};
std::string                    DrawableFrame::cp_vshader    = "\
#version 330 core                       \n\
layout (location = 0) in vec2 aPos;     \n\
layout (location = 1) in vec2 aUV;      \n\
out vec2 uv;                            \n\
void main() {                           \n\
    gl_Position = vec4(aPos, 0.0, 1.0); \n\
    uv = aUV;                           \n\
}";
std::string                    DrawableFrame::cp_fshader    = "\
#version 330 core                       \n\
in vec2 uv;                             \n\
out vec4 FragColor;                     \n\
uniform sampler2D texture0;             \n\
void main() {                           \n\
    FragColor = texture(texture0, uv);  \n\
}";
std::shared_ptr<ShaderProgram> DrawableFrame::cp_prog{};

DrawableFrame::DrawableFrame(
    GLuint width, GLuint height, bool require_color_buffer, bool require_depth_buffer
) :
    FrameBufferObject(width, height, require_color_buffer, require_depth_buffer),
    cur_rect_(0, 0, width, height) {

    // utilities init parms and prog
    MY_CHECK_FAIL
    if (nb_inst_ == 0) {
        cp_vao.emplace();
        cp_vbo.emplace();
        cp_ebo.emplace(GL_ELEMENT_ARRAY_BUFFER);
        cp_prog = std::make_shared<ShaderProgram>(cp_vshader, cp_fshader);

        MY_CHECK_FAIL
        cp_vao->bind();

        cp_vbo->SetBufferData(sizeof(cp_vertices), cp_vertices);

        // Position attribute (location = 0)
        cp_vbo->SetAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);

        // UV attribute (location = 1)
        cp_vbo->SetAttribPointer(
            1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float))
        );
        MY_CHECK_FAIL
        // ebo
        cp_ebo->SetBufferData(sizeof(cp_indices), cp_indices);
        // unbind
        cp_vao->unbind();
    }
    nb_inst_++;
    MY_CHECK_FAIL
}

DrawableFrame::~DrawableFrame() {
    MY_CHECK_FAIL
    nb_inst_--;
    if (nb_inst_ == 0) {
        cp_vao.reset();
        cp_vbo.reset();
        cp_ebo.reset();
        cp_prog.reset();
    }
}

void DrawableFrame::draw(bool to_frame) {
    if (to_frame) {
        unbind(); // not necessary. suppose draw target is bound outside
    }
    cp_prog->use();
    cp_vao->bind();
    tex0_->activate_sampler(cp_prog, "texture0");
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

// bind+viewport
void DrawableFrame::viewport(mf::Rect rect) {
    bind();

    rect = get_draw_rect(rect);
    validate_rect(rect);
    glViewport(rect.x, height_ - rect.y - rect.h, rect.w, rect.h);
    MY_CHECK_FAIL
}

// bind+viewport+clear
void DrawableFrame::clear_color(mf::Rect rect, GLenum bits, glm::u8vec4 color) {
    viewport(rect); // bind+viewport
    glClearColor(color.x, color.y, color.z, color.w);
    MY_CHECK_FAIL

    // scissor clear
    rect = get_draw_rect(rect);
    glEnable(GL_SCISSOR_TEST);
    glScissor(rect.x, height_ - rect.y - rect.h, rect.w, rect.h);
    glClear(bits);
    glDisable(GL_SCISSOR_TEST);
    MY_CHECK_FAIL
}

void DrawableFrame::paste_tex(std::shared_ptr<TextureObject> tex, mf::Rect rect) {
    clear_color(rect); // bind+viewport+clear
    // paste tex
    cp_prog->use();
    cp_vao->bind();
    tex->activate_sampler(cp_prog, "texture0");
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void DrawableFrame::paste_fbo(FrameBufferObject &fbo2, mf::Rect rect) {
    auto tex = fbo2.tex0();
    paste_tex(tex, rect); // clear+paste_tex
}

mf::Rect DrawableFrame::get_draw_rect(mf::Rect r) {
    return mf::Rect(
        (r.x - cur_rect_.x) * (int)width_ / cur_rect_.w,
        (r.y - cur_rect_.y) * (int)height_ / cur_rect_.h, r.w * (int)width_ / cur_rect_.w,
        r.h * (int)height_ / cur_rect_.h
    );
}

void DrawableFrame::validate_rect(mf::Rect rect) {
    if (!mf::Rect(0, 0, width_ + 1, height_ + 1).contains(rect)) {
        spdlog::warn("FBO::VALIDATE_RECT: viewport out of range (relative to tex size)");
        spdlog::warn(
            "{},{} !contains {},{},{},{}", width_, height_, rect.x, rect.y, rect.w, rect.h
        );
        // exit(-1);
    }
}