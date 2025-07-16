#pragma once

#include "buffer_objects.hxx"
#include "config.hxx"
#include "shader.hxx"
#include "shader_program.hxx"
#include "texture_objects.hxx"
#include "utils.hxx"

namespace mf {
    using glwrapper::BufferObject;
    using glwrapper::FrameBufferObject;
    using glwrapper::ShaderProgram;
    using glwrapper::TextureObject;
    using glwrapper::VertexArrayObject;
    using glwrapper::VertexBufferObject;

    class DrawableFrame : public FrameBufferObject {
        public:
        DrawableFrame(
            GLuint width = DEFAULT_FBO_WIDTH, GLuint height = DEFAULT_FBO_HEIGHT,
            bool require_color_buffer = true, bool require_depth_buffer = true
        );
        DrawableFrame &operator=(DrawableFrame &&o);
        inline ~DrawableFrame() { cleanup(); };
        void cleanup();

        // draw utilities

        void draw(bool to_frame = true);

        mf::Rect viewport(mf::Rect rect);
        void     clear_color(
                mf::Rect rect, GLenum bits = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
                glm::u8vec4 color = DEFAULT_CLEAR_COLOR
            );
        void paste_tex(std::shared_ptr<TextureObject> tex, mf::Rect rect);
        void paste_fbo(FrameBufferObject &fbo2, mf::Rect rect);

        // rect pasting logic

        // get the rect relative to texture
        mf::Rect get_draw_rect(mf::Rect r);

        // set output rect relative to screen
        void set_cur_rect(mf::Rect rect);

        // resize tex0
        inline void resize(int w, int h) {
            assert(false && "fbo resize unimplemented");
            tex0_->from_data(nullptr, w, h);
            width_ = w, height_ = h;
        }

        void validate_rect(mf::Rect rect);

        protected:
        // the output rect relative to screen
        mf::Rect cur_rect_;

        // static draw tools
        static std::optional<VertexArrayObject>  cp_vao;
        static std::optional<VertexBufferObject> cp_vbo;
        static std::optional<BufferObject>       cp_ebo;

        static int nb_inst_;

        static float                          cp_vertices[16];
        static GLuint                         cp_indices[6];
        static std::string                    cp_vshader;
        static std::string                    cp_fshader;
        static std::shared_ptr<ShaderProgram> cp_prog; // unique
    };
} // namespace mf