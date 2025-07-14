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
    spdlog::debug("genbuffer (id:{})", ID_);
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

    spdlog::debug("deleting buffer(type={},id={})", buffer_type_, ID_);
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
    GLuint width, GLuint height, bool require_color_buffer, bool require_depth_buffer
) :
    width_(width),
    height_(height) {
    // init buffer
    glGenFramebuffers(1, &ID_);

    // bind
    bind();
    // associate tex
    if (require_color_buffer) {
        tex0_ = std::make_shared<TextureObject>("texture0");
        tex0_->from_data(nullptr, width_, height_);
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex0_->ID(),
            0 // no mipmap
        );
    } else { // select NONE color att for fbo
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }
    // associate depth buffer
    if (require_depth_buffer) {
        tex_depth_ = std::make_shared<TextureObject>(
            "texture_depth", 0, TextureParameter(), GL_DEPTH_COMPONENT
        );
        tex_depth_->from_data(nullptr, width_, height_);
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex_depth_->ID(), 0
        );
    }
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        spdlog::error("framebuffer incomplete");
        exit(-1);
    }
    // unbind
    unbind();

} // fbo constructor/>

FrameBufferObject::FrameBufferObject(FrameBufferObject &&o) :
    ID_(o.ID_), tex0_(std::move(o.tex0_)), tex_depth_(std::move(o.tex_depth_)), width_(o.width_),
    height_(o.height_) {
    o.ID_ = 0;
}

FrameBufferObject::~FrameBufferObject() {
    MY_CHECK_FAIL
    if (ID_ == 0) return;

    spdlog::info("FrameBufferObject::~FrameBufferObject (id={})", ID_);
    glDeleteFramebuffers(1, &ID_);
    MY_CHECK_FAIL

    spdlog::info("deleted frame buffer(id={})", ID_);
}

void FrameBufferObject::bind() {
    MY_CHECK_FAIL
    glBindFramebuffer(GL_FRAMEBUFFER, ID_);
}
