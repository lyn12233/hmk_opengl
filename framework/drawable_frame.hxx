#pragma once

#include "buffer_objects.hxx"
#include "utils.hxx"

namespace mf {
    class DrawableFrame : public FrameBufferObject {
        public:
        DrawableFrame(
            GLuint width = 800, GLuint height = 600, bool require_color_buffer = true,
            bool require_depth_buffer = true
        );
        ~DrawableFrame();

        // draw utilities

        glm::vec4 bgcolor{.1, .1, .1, .1};

        void draw(bool to_frame = true);

        void viewport(mf::Rect rect);
        void clear_color(
            mf::Rect rect, GLenum bits = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
            glm::u8vec4 color = {0, 0, 0, 0}
        );
        void paste_tex(std::shared_ptr<TextureObject> tex, mf::Rect rect);
        void paste_fbo(FrameBufferObject &fbo2, mf::Rect rect);

        // rect pasting logic

        // get the rect relative to texture
        mf::Rect get_draw_rect(mf::Rect r);

        // set output rect relative to screen
        inline void set_cur_rect(mf::Rect rect) { cur_rect_ = rect; }

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