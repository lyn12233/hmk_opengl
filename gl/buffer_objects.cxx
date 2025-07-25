#include "buffer_objects.hxx"

#include "checkfail.hxx"
#include "shader_program.hxx"
#include "texture_objects.hxx"

#include <memory>
#include <optional>

#include <spdlog/spdlog.h>

using glwrapper::BufferObject;
using glwrapper::FrameBufferObject;
using glwrapper::VertexArrayObject;
using glwrapper::VertexBufferObject;

BufferObject::BufferObject(GLenum buffer_type) : buffer_type_(buffer_type) {
    glGenBuffers(1, &ID_);
    spdlog::debug("BufferObject::BufferObject(id={})", ID_);
}

BufferObject::BufferObject(BufferObject &&o) : buffer_type_(o.buffer_type_), ID_(o.ID_) {
    o.ID_ = 0;
}

void BufferObject::operator=(BufferObject &&o) {
    if (this != &o) {
        cleanup();
        ID_   = o.ID_;
        o.ID_ = 0;
    }
}

void BufferObject::cleanup() {
    MY_CHECK_FAIL
    if (ID_ == 0) return;

    spdlog::debug("BufferObject::cleanup(type={},id={})", buffer_type_, ID_);
    validate();
    glDeleteBuffers(1, &ID_);
    MY_CHECK_FAIL
}

void BufferObject::bind() {
    MY_CHECK_FAIL
    glBindBuffer(buffer_type(), ID());
}

void BufferObject::validate() {
    if (glIsBuffer(ID()) != GL_TRUE) {
        spdlog::error("operating on invalid buffer(type={},id={})", buffer_type_, ID_);
        exit(-1);
    }
}

void BufferObject::SetBufferData(size_t size, const void *data, GLenum usage) {
    bind();
    glBufferData(buffer_type(), size, data, usage);
}

// vertex buffer object

VertexBufferObject::VertexBufferObject() : BufferObject(GL_ARRAY_BUFFER) {}

VertexBufferObject::~VertexBufferObject() { // todo: delete vbo//unecessary
    spdlog::debug("VertexBufferObject::~VertexBufferObject()");
}

void VertexBufferObject::SetAttribPointer(
    GLuint      index,     // attribute location
    int         size,      // input item dimension
    GLenum      type,      // input value type
    bool        normalize, // whether to noramlize, to [-1,1] or [0,1]
    int         stride,    // offset between consecutive attributes, =0:tightly packed
    const void *pointer    // offset of the first attribute
) {
    bind(); // bind and validate
    glVertexAttribPointer(index, size, type, normalize, stride, pointer);
    glEnableVertexAttribArray(index);
}

void VertexBufferObject::SetAttribIPointer(
    GLuint index, int size, GLenum type, int stride, const void *pointer
) {
    bind();
    glVertexAttribIPointer(index, size, type, stride, pointer);
    glEnableVertexAttribArray(index);
}

// vertex array object

VertexArrayObject::VertexArrayObject() { glGenVertexArrays(1, &ID_); }

VertexArrayObject::VertexArrayObject(VertexArrayObject &&o) : ID_(o.ID_) { o.ID_ = 0; }

void VertexArrayObject::operator=(VertexArrayObject &&o) {
    if (this != &o) {
        cleanup();
        ID_   = o.ID_;
        o.ID_ = 0;
    }
}

void VertexArrayObject::cleanup() {
    MY_CHECK_FAIL
    if (ID_ == 0) return;
    glDeleteVertexArrays(1, &ID_);
}

void VertexArrayObject::bind() {
    MY_CHECK_FAIL
    glBindVertexArray(ID_);
}

// framebuffer

FrameBufferObject::FrameBufferObject(
    GLuint width, GLuint height, int nb_color_attachment /*=1*/, bool require_depth_buffer /*=true*/
) :
    width_(width),
    height_(height) // unused?
{
    // init buffer
    glGenFramebuffers(1, &ID_);

    // bind
    bind();
    // associate tex
    nb_color_attachment = std::max(nb_color_attachment, 0);

    if (nb_color_attachment > 0) {
        for (int i = 0; i < nb_color_attachment; i++) {
            spdlog::debug("FrameBufferObject::FrameBufferObject: gen color att ({})", i);

            auto tex = std::make_shared<TextureObject>( //
                "", 0, TextureParameter("discrete"), GL_RGBA32F
            );
            tex->from_data(nullptr, width_, height_);

            glFramebufferTexture2D(
                GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, tex->ID(), 0
            );

            color_attachments.push_back(tex);
        }
    } else {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    // associate depth buffer
    if (require_depth_buffer) {
        spdlog::debug("FrameBufferObject::FrameBufferObject: gen depth att");

        tex_depth_ =
            std::make_shared<TextureObject>("", 0, TextureParameter(), GL_DEPTH_COMPONENT24);
        tex_depth_->from_data(nullptr, width_, height_);

        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex_depth_->ID(), 0
        );
    }

    // check completeness
    validate();

    // unbind
    unbind();

} // fbo constructor/>

FrameBufferObject::FrameBufferObject(FrameBufferObject &&o) :
    ID_(o.ID_), color_attachments(std::move(o.color_attachments)),
    tex_depth_(std::move(o.tex_depth_)), width_(o.width_), height_(o.height_) {
    o.ID_ = 0;
}

void FrameBufferObject::cleanup() {
    MY_CHECK_FAIL
    if (ID_ == 0) return;

    spdlog::info("FrameBufferObject::~FrameBufferObject (id={})", ID_);
    glDeleteFramebuffers(1, &ID_);
    MY_CHECK_FAIL

    spdlog::info("deleted frame buffer(id={})", ID_);
}

FrameBufferObject &FrameBufferObject::operator=(FrameBufferObject &&o) {
    if (this != &o) {
        cleanup();

        ID_     = o.ID_;
        width_  = o.width_;
        height_ = o.height_;

        color_attachments = std::move(o.color_attachments);
        tex_depth_        = std::move(o.tex_depth_);

        o.ID_ = 0;
    }
    return *this;
}

void FrameBufferObject::validate() {
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        spdlog::error("framebuffer incomplete");
        exit(-1);
    }
}

void FrameBufferObject::bind() {
    MY_CHECK_FAIL
    glBindFramebuffer(GL_FRAMEBUFFER, ID_);
}
