#pragma once

#include "shader_program.hxx"
#include "texture_objects.hxx"

#include <iostream>
#include <memory>
#include <optional>
#include <string>

#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

#ifndef __gl_h_
    #include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>

class BufferObject {
    public:
    BufferObject(GLenum buffer_type);
    BufferObject(BufferObject &&o);
    BufferObject(const BufferObject &) = delete;
    ~BufferObject();

    void bind();
    void validate();

    void SetBufferData(size_t size, const void *data, GLenum usage = GL_STATIC_DRAW);

    // readonly's
    inline auto buffer_type() { return buffer_type_; };
    inline auto ID() { return ID_; };

    protected:
    GLenum buffer_type_;
    GLuint ID_;
};

class VertexBufferObject : public BufferObject {
    public:
    VertexBufferObject();
    ~VertexBufferObject();

    // set shader in-parms, float type, data from current buffer
    void SetAttribPointer(
        GLuint      index,             // attribute location
        int         size,              // input item dimension
        GLenum      type,              // input value type
        bool        normalize = false, // whether to noramlize, to [-1,1](signed) or [0,1](unsigned)
        int         stride    = 0,     // offset between consecutive attributes, =0:tightly packed
        const void *pointer   = NULL   // offset of the first attribute
    );
    // set shader in-parms, int type
    void SetAttribIPointer(
        GLuint index, int size, GLenum type, int stride = 0, const void *pointer = NULL
    );
};

class VertexArrayObject {
    public:
    VertexArrayObject();
    VertexArrayObject(VertexArrayObject &&o);
    VertexArrayObject(const VertexArrayObject &) = delete;
    ~VertexArrayObject();

    void bind();
    void inline unbind() { glBindVertexArray(0); }

    private:
    GLuint ID_;
};

class FrameBufferObject {
    public:
    FrameBufferObject(
        GLuint width = 800, GLuint height = 600, bool require_color_buffer = true,
        bool require_depth_buffer = true
    );
    FrameBufferObject(FrameBufferObject &&o);
    FrameBufferObject(const FrameBufferObject &) = delete;
    ~FrameBufferObject();

    void bind();
    void inline unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

    // // draw utilities
    // glm::vec4 bgcolor{.1,.1,.1,.1};
    //
    // void draw(bool to_frame=true);

    // void viewport(mf::Rect rect);
    // void clear_color(mf::Rect rect, GLenum bits =
    // GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); void
    // paste_tex(std::shared_ptr<TextureObject> tex,mf::Rect rect); void
    // paste_fbo(FrameBufferObject& fbo2,mf::Rect rect);
    //
    // // get the rect relative to texture
    // mf::Rect get_draw_rect(mf::Rect r);

    // readonly's
    inline auto tex0() { return tex0_; } // not necessary

    // //set output rect relative to screen
    // inline void set_cur_rect(mf::Rect rect){cur_rect_=rect;}

    inline auto width() { return width_; }
    inline auto height() { return height_; }

    protected:
    GLuint                         ID_;
    std::shared_ptr<TextureObject> tex0_;
    std::shared_ptr<TextureObject> tex_depth_;

    // size of framebuffer
    GLuint width_;
    GLuint height_;

    // //the output rect relative to screen
    // mf::Rect cur_rect_;

    // // statics
    // static std::optional<VertexArrayObject> cp_vao;
    // static std::optional<VertexBufferObject> cp_vbo;
    // static std::optional<BufferObject> cp_ebo;  //default index type
    // // unisgned int
    //
    // static int nb_inst_;
    //
    // static float cp_vertices[16];
    // static GLuint cp_indices[6];
    // static std::string cp_vshader;
    // static std::string cp_fshader;
    // static std::shared_ptr<ShaderProgram> cp_prog;//unique
    //
    // void validate_rect(mf::Rect rect);
};
