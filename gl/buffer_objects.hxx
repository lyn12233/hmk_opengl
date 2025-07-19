#pragma once

#include "config.hxx"
#include "shader_program.hxx"
#include "texture_objects.hxx"

#include <iostream>
#include <memory>
#include <optional>
#include <string>

#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include <vector>

#ifndef __gl_h_
    #include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>

///@addtogroup gl_wrappers
///@{

namespace glwrapper {

    /// @brief buffer wrapper for glGenBuffers and glDeleteBuffers
    class BufferObject {

        public:
        ///@brief constructor
        ///@param buffer_type type of buffer
        BufferObject(GLenum buffer_type);
        BufferObject(BufferObject &&o);
        void operator=(BufferObject &&o);
        BufferObject(const BufferObject &) = delete;
        inline ~BufferObject() { cleanup(); };
        void cleanup();

        ///@brief wrapper for glBindBuffer
        void bind();

        ///@ingroup gl_wrappers
        ///@brief wrapper for glIsBuffer, abort if failed
        void validate();

        ///@brief wrapper for glBufferData
        ///@param size size of data in bytes
        ///@param data pointer to data
        ///@param usage buffer usage, default to GL_STATIC_DRAW
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
        inline void operator=(VertexBufferObject &&o) { BufferObject::operator=(std::move(o)); };

        // set shader in-parms, float type, data from current buffer
        void SetAttribPointer(
            GLuint index,              // attribute location
            int    size,               // input item dimension
            GLenum type,               // input value type
            bool   normalize = false,  // whether to noramlize, to [-1,1](signed) or [0,1](unsigned)
            int    stride    = 0,      // offset between consecutive attributes, =0:tightly packed
            const void *pointer = NULL // offset of the first attribute
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
        void operator=(VertexArrayObject &&o);
        inline ~VertexArrayObject() { cleanup(); };
        void cleanup();

        void bind();
        void inline unbind() { glBindVertexArray(0); }

        private:
        GLuint ID_;
    };

    class FrameBufferObject {
        public:
        FrameBufferObject(
            GLuint width = DEFAULT_FBO_WIDTH, GLuint height = DEFAULT_FBO_HEIGHT,
            int nb_color_attachment = 1, bool require_depth_buffer = true
        );
        FrameBufferObject(FrameBufferObject &&o);
        FrameBufferObject(const FrameBufferObject &) = delete;
        FrameBufferObject &operator=(FrameBufferObject &&o);
        inline ~FrameBufferObject() { cleanup(); };
        void cleanup();

        /// @brief validate fbo completeness
        void validate();

        void bind();
        void inline unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

        // readonly's
        inline auto tex0() { return color_attachments[0]; } // not necessary
        inline auto tex(int i) { return color_attachments[i]; }

        inline auto width() { return width_; }
        inline auto height() { return height_; }

        protected:
        GLuint                                      ID_;
        std::vector<std::shared_ptr<TextureObject>> color_attachments;
        std::shared_ptr<TextureObject>              tex_depth_;

        // size of framebuffer
        GLuint width_;
        GLuint height_;
    };

} // namespace glwrapper

///@}
// end of @group