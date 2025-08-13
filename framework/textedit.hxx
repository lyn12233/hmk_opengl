#pragma once

#include "buffer_objects.hxx"
#include "config.hxx"
#include "shader_program.hxx"
#include "texture_objects.hxx"
#include "utils.hxx"
#include "widget.hxx"

#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

namespace mf {
    // ' ' - '~' ascii char texture of given font size
    class AsciiTex : public TextureObject {
        public:
        AsciiTex(GLuint tex_width = DEFAULT_TC_HEIGHT, std::string font_path = DEFAULT_TTF_PATH);
        ~AsciiTex();

        //----type: 8UC4
        // cv::Mat tex_data_;
        std::vector<glm::u8vec4> tex_data;

        GLuint tex_width_;
        GLuint tex_height_;

        constexpr static char tex_begin = ' ';
        constexpr static char tex_end   = '~' + 1;
        constexpr static char tex_nb    = '~' - ' ' + 1;
    };

    // general single-line ascii text logic, handles color, insert, delete logic
    class TextEdit {
        public:
        TextEdit(bool allow_trailing = true);
        ~TextEdit();

        void validate();

        void on_insert(char c);
        void on_insert(std::string s);
        void on_delete_();
        void on_backspace();
        void on_mouse_up(GLuint at);
        void on_mouse_down(GLuint at);
        void on_mouse_move(GLuint at);
        void on_left();
        void on_right();
        void on_select_all();

        std::vector<char> get_color_masks();

        inline std::string get_text() { return has_trailing() ? text_ + ' ' : text_; }
        inline void        set_text(std::string text) { text_ = text, cur_pos_ = 0, last_pos_ = 0; }
        inline GLuint      size() { return text_.size() + has_trailing(); }

        inline bool has_trailing() {
            return cur_pos_ == text_.size() && allow_trailing || text_.size() == 0;
        }
        bool allow_trailing;

        protected:
        void        delete_range();
        std::string text_;
        GLuint      cur_pos_;  // selection end pos, incuded
        GLuint      last_pos_; // selection start pos, included
        bool        mouse_down_;
    };

    class TextCtrl : public WidgetBase {
        public:
        TextCtrl(
            std::string text = "", GLuint w = DEFAULT_TC_WIDTH, GLuint h = DEFAULT_TC_HEIGHT,
            GLuint    fontsize = DEFAULT_TC_FONTSIZE,
            mf::FLAGS style    = static_cast<FLAGS>(mf::ALIGN_CENTER | mf::EXPAND)
        );
        ~TextCtrl() override;

        bool draw(DrawableFrame &fbo) override;
        void event_at(EVENT evt, Pos at, EVENT_PARM parameter) override;

        // buffer
        VertexArrayObject      vao;
        VertexBufferObject     vbo;
        std::vector<int>       vbo_data; //(x,y,u,v,color_type)
        BufferObject           ebo;
        std::vector<int>       ebo_data;
        std::vector<glm::vec4> colors; //[<bkgd>,<frgd>,]+

        // text infos and attrs

        inline std::string get_text() { return editor.get_text(); }
        inline void set_text(std::string text) { editor.set_text(text), mark_dirty(false, true); }

        GLuint fontsize_;
        // given cur_rect, style, texteditor, set x/ytext offset relative to screen
        void update_text_coord();
        // evaluate font width to screen
        inline float eval_font_width() {
            return (float)fontsize_ * tex_->tex_width_ / (float)tex_->tex_height_;
        }
        // evaluate text width to screen
        inline float eval_text_width() { return (float)editor.size() * eval_font_width(); }
        // evaluate cursor pos on text
        inline GLuint eval_cursor_pos(int x /*rel to scr*/) {
            int pos = (int)((float)(x - xtext) / eval_font_width());
            return (pos < 0) ? 0 : ((pos >= editor.size()) ? editor.size() - 1 : pos);
        }
        int xtext;
        int ytext;
        int wtext;
        int htext;

        TextEdit editor;

        // statics
        static std::shared_ptr<AsciiTex>      tex_;
        static std::string                    vshader;
        static std::string                    fshader;
        static std::shared_ptr<ShaderProgram> prog;
    };

    class StaticText : public TextCtrl {
        public:
        StaticText(
            std::string text = "", GLuint w = DEFAULT_TC_WIDTH, GLuint h = DEFAULT_TC_HEIGHT,
            GLuint    fontsize = DEFAULT_TC_FONTSIZE,
            mf::FLAGS style    = static_cast<FLAGS>(mf::ALIGN_CENTER | mf::EXPAND)
        );
        ~StaticText() override;
        void event_at(EVENT evt, Pos at, EVENT_PARM parameter) override;
    };
} // namespace mf