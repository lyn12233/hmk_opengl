#include "textedit.hxx"
#include "checkfail.hxx"
#include "shader_program.hxx"
#include "stb_truetype.h"
#include "utils.hxx"
#include "widget.hxx"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <memory>
#include <vector>

#include <GLFW/glfw3.h>
#include <glm/fwd.hpp>
#include <spdlog/spdlog.h>

using mf::AsciiTex;
using mf::TextCtrl;
using mf::TextEdit;

// local utils
namespace mf {
    namespace te_utils {
        bool load_font(
            const std::string          &font_path /*path to .ttf*/,
            std::vector<unsigned char> &font_buffer /*file raw data*/
        ) {
            std::ifstream ifs(font_path, std::ios::binary);
            if (!ifs) return false;
            font_buffer = std::vector<unsigned char>(std::istreambuf_iterator<char>(ifs), {});
            return true;
        }
    } // namespace te_utils
} // namespace mf

AsciiTex::AsciiTex(GLuint tex_height, std::string font_path) {
    // load font
    std::vector<unsigned char> font_data;
    if (!mf::te_utils::load_font(font_path, font_data)) {
        spdlog::error("cant load font ({})", font_path);
        exit(-1);
    }
    // init stb font
    stbtt_fontinfo font;
    if (!stbtt_InitFont(&font, font_data.data(), 0)) {
        spdlog::error("can't init font");
        exit(-1);
    }

    // scale
    float scale = stbtt_ScaleForMappingEmToPixels(&font, tex_height);

    // ascent and scale; for courier new, they are fixed
    int ascent, descent, advance;
    stbtt_GetFontVMetrics(&font, &ascent, &descent, nullptr); // gap is always zero
    stbtt_GetCodepointHMetrics(&font, 'a', &advance, nullptr);
    ascent  = std::round(ascent * scale);
    descent = std::round(descent * scale);
    advance = std::round(advance * scale);

    // spdlog::debug("ascent:{},descent:{},davance:{}",ascent,descent,advance);

    // img data
    //  cv::Mat img_data(tex_height,advance*('~'-' '+1),CV_8UC1,cv::Scalar(1));
    int nb_rows = tex_height;
    int nb_cols = advance * tex_nb;
    tex_data    = std::vector<glm::u8vec4>(nb_rows * nb_cols, glm::uvec4(0));

    int xcursor = 0;

    for (int i = ' '; i <= '~'; i++) {

        // int glyph = stbtt_FindGlyphIndex(&font, i);
        int glyph = i;

        int            w, h, xoff, yoff;
        unsigned char *bitmap =
            stbtt_GetCodepointBitmap(&font, scale, scale, glyph, &w, &h, &xoff, &yoff);
        int ystart = ascent + yoff;

        // left side bearing
        int lsb;
        stbtt_GetCodepointHMetrics(&font, i, nullptr, &lsb);
        lsb        = std::round(lsb * scale);
        int xstart = xcursor + lsb;

        // spdlog::debug("{}({},{},{},{})->{}",(char)glyph,xoff,ystart,w,h,lsb);

        for (int row = 0; row < h; row++) {
            for (int col = 0; col < w; col++) {
                int xdst = xstart + col;
                int ydst = ystart + row;
                if (xdst >= 0 && xdst < nb_cols && ydst >= 0 && ydst < nb_rows) {
                    char val = bitmap[row * w + col];
                    // auto& px = img_data.at<uchar>(ydst,xdst);
                    // px = val;
                    tex_data[ydst * nb_cols + xdst] = glm::u8vec4(val, val, val, 255);
                } // drawpixel/>
            }     // iter col/>
        }         // iter row/>

        stbtt_FreeBitmap(bitmap, nullptr);

        // calc davance
        xcursor += advance;
    } // iter char/>

    // cv::cvtColor(img_data, tex_data_, cv::COLOR_GRAY2RGBA);
    // cv::imshow("ASCII Strip", tex_data_);
    // cv::waitKey(0);

    from_data(reinterpret_cast<GLuint *>(tex_data.data()), nb_cols, nb_rows);

    tex_height_ = tex_height;
    tex_width_  = advance;
}
AsciiTex::~AsciiTex() {}

TextEdit::TextEdit() {
    cur_pos_    = 0;
    last_pos_   = 0;
    mouse_down_ = false;
}

TextEdit::~TextEdit() {}

void TextEdit::validate() {
    if (cur_pos_ < 0 || cur_pos_ > text_.size() || last_pos_ < 0 || last_pos_ > text_.size()) {
        spdlog::error("invalid selection");
        exit(-1);
    }
}
void TextEdit::on_insert(char c) {
    validate();
    delete_range();
    c = (AsciiTex::tex_begin <= c && c < AsciiTex::tex_end) ? c : ((c == '\0' ? ' ' : '?'));
    text_.insert(cur_pos_, 1, c);
    cur_pos_++;
    last_pos_ = cur_pos_;
}
void TextEdit::on_insert(std::string s) {
    for (auto c : s) {
        on_insert(c);
    }
}
void TextEdit::on_delete_() {
    validate();
    if (cur_pos_ != last_pos_) {
        delete_range();
    } else if (cur_pos_ != text_.size()) {
        text_.erase(cur_pos_, 1);
    }
}
void TextEdit::on_backspace() {
    validate();
    if (cur_pos_ != last_pos_) {
        delete_range();
    } else if (cur_pos_ != 0) {
        cur_pos_--;
        text_.erase(cur_pos_, 1);
        last_pos_ = cur_pos_;
    }
}
void TextEdit::on_mouse_up(GLuint at) { mouse_down_ = false; }
void TextEdit::on_mouse_down(GLuint at) {
    mouse_down_ = true;
    last_pos_   = at;
    cur_pos_    = at;
    validate();
}
void TextEdit::on_mouse_move(GLuint at) {
    if (mouse_down_) {
        cur_pos_ = at;
        validate();
    }
}
void TextEdit::on_right() {
    validate();
    cur_pos_ = cur_pos_ > last_pos_ ? cur_pos_ : last_pos_;
    if (cur_pos_ != text_.size()) {
        cur_pos_++;
    }
    last_pos_ = cur_pos_;
}
void TextEdit::on_left() {
    validate();
    cur_pos_ = cur_pos_ < last_pos_ ? cur_pos_ : last_pos_;
    if (cur_pos_ != 0) {
        cur_pos_--;
    }
    last_pos_ = cur_pos_;
}

std::vector<char> TextEdit::get_color_masks() {
    validate();
    std::vector<char> mask(text_.size() + 1, 0);
    if (cur_pos_ <= last_pos_) {
        std::fill(mask.begin() + cur_pos_, mask.begin() + last_pos_ + 1, 1);
    } else {
        std::fill(mask.begin() + last_pos_, mask.begin() + cur_pos_ + 1, 1);
    }
    return mask;
}

void TextEdit::delete_range() {
    if (cur_pos_ != last_pos_) {
        GLuint min_pos = cur_pos_ < last_pos_ ? cur_pos_ : last_pos_;
        GLuint max_pos = cur_pos_ > last_pos_ ? cur_pos_ : last_pos_;
        text_.erase(min_pos, max_pos - min_pos);
        last_pos_ = min_pos;
        cur_pos_  = min_pos;
    }
}

TextCtrl::TextCtrl(std::string text, GLuint w, GLuint h, GLuint fontsize, mf::FLAGS style)
    : WidgetBase(w, h, {}, style),
      // cur_rect_(0,0,w,h),
      fontsize_(fontsize), xtext(0), ytext(0),
      ebo(GL_ELEMENT_ARRAY_BUFFER), colors{{1, 1, 1, 1}, {0, 0, 1, 1}, {0, 1, 0, 1}, {1, 0, 0, 1}} {
    if (!tex_) {
        tex_ = std::make_shared<AsciiTex>(fontsize);
    }
    if (!prog) {
        prog = std::make_shared<ShaderProgram>("", "", vshader, fshader);
    }
    editor.on_insert(text);

    focus_ = false;

    dirty = true;
}
TextCtrl::~TextCtrl() { spdlog::debug("TextCtrl::~TextCtrl()"); }

// statics
std::shared_ptr<AsciiTex>      TextCtrl::tex_{};
std::shared_ptr<ShaderProgram> TextCtrl::prog{};
std::string                    TextCtrl::vshader = "\
#version 330 core \n\
layout (location=0) in int x;\n\
layout (location=1) in int y;\n\
layout (location=2) in int u;\n\
layout (location=3) in int v;\n\
layout (location=4) in int c;\n\
layout (location=5) in int mask;\n\
uniform int len_text;\n\
uniform int len_tex;\n\
uniform int start_tex;\n\
uniform vec4 colors[4];\n\
\n\
out vec2 tex_coord;\n\
out vec4 frcolor;\n\
out vec4 bkcolor;\n\
\n\
void main(){\n\
    float rx,ry;        \n\
    rx=float(x)/float(len_text);\n\
    rx=rx*2-1;\n\
    ry=y*2-1;\n\
    tex_coord.x=float(c-start_tex+u)/float(len_tex);\n\
    tex_coord.y=1-v;\n\
    frcolor=colors[2*mask];\n\
    bkcolor=colors[2*mask+1];\n\
    gl_Position=vec4(rx,ry,0,1);\n\
}    \n\
";
std::string                    TextCtrl::fshader = "\
#version 330 core   \n\
in vec2 tex_coord;\n\
in vec4 frcolor;\n\
in vec4 bkcolor;\n\
out vec4 FragColor;\n\
uniform sampler2D texture0;\n\
void main(){\n\
    FragColor = mix(frcolor,bkcolor,texture(texture0,tex_coord).x);\n\
}\n\
";

bool TextCtrl::draw(DrawableFrame &fbo) {
    if (!dirty) return false;
    dirty = false;

    // collect data
    spdlog::debug("TextCtrl::draw setting data");
    int  len  = editor.size();
    auto str  = editor.get_text();
    auto mask = editor.get_color_masks();
    if (!focus_) {
        std::fill(mask.begin(), mask.end(), 0);
    }

    ebo_data.clear();
    vbo_data.clear();
    for (int i = 0; i < len; i++) {
        vbo_data.insert(vbo_data.end(), {i, 0, 0, 0, static_cast<int>(str[i]), mask[i]});
        vbo_data.insert(vbo_data.end(), {i + 1, 0, 1, 0, static_cast<int>(str[i]), mask[i]});
        vbo_data.insert(vbo_data.end(), {i, 1, 0, 1, static_cast<int>(str[i]), mask[i]});
        vbo_data.insert(vbo_data.end(), {i + 1, 1, 1, 1, static_cast<int>(str[i]), mask[i]});
        ebo_data.insert(
            ebo_data.end(), {4 * i, 4 * i + 1, 4 * i + 2, 4 * i + 3, 4 * i + 2, 4 * i + 1}
        );
    }
    // exit(-1);

    // set attr
    vao.bind();
    vbo.bind();
    vbo.SetBufferData(vbo_data.size() * sizeof(int), vbo_data.data());
    vbo.SetAttribIPointer(0, 1, GL_INT, 6 * sizeof(int), 0);
    vbo.SetAttribIPointer(1, 1, GL_INT, 6 * sizeof(int), (void *)sizeof(int));
    vbo.SetAttribIPointer(2, 1, GL_INT, 6 * sizeof(int), (void *)(sizeof(int) * 2));
    vbo.SetAttribIPointer(3, 1, GL_INT, 6 * sizeof(int), (void *)(sizeof(int) * 3));
    vbo.SetAttribIPointer(4, 1, GL_INT, 6 * sizeof(int), (void *)(sizeof(int) * 4));
    vbo.SetAttribIPointer(5, 1, GL_INT, 6 * sizeof(int), (void *)(sizeof(int) * 5));
    MY_CHECK_FAIL
    ebo.bind();
    ebo.SetBufferData(ebo_data.size() * sizeof(int), ebo_data.data());
    MY_CHECK_FAIL
    // vao.unbind();
    prog->use();
    prog->set_value("len_text", len);
    prog->set_value("len_tex", (int)AsciiTex::tex_nb);
    prog->set_value("start_tex", (int)AsciiTex::tex_begin);
    prog->set_value("colors", colors);

    // draw to screen
    update_text_coord();
    // spdlog::debug("text coord: {},{},{},{}",xtext,ytext,wtext,htext);

    fbo.clear_color(cur_rect, GL_COLOR_BUFFER_BIT, {255, 255, 0, 255});
    fbo.viewport(Rect(xtext, ytext, wtext, htext));
    MY_CHECK_FAIL
    tex_->activate_sampler(prog, "texture0", 0);
    MY_CHECK_FAIL
    glDrawElements(GL_TRIANGLES, ebo_data.size(), GL_UNSIGNED_INT, 0);
    MY_CHECK_FAIL
    vao.unbind();
    fbo.unbind();

    return false;
}

void TextCtrl::event_at(EVENT evt, Pos at, EVENT_PARM parameter) {

    if (evt == EVT_RESIZE) {
        mark_dirty(false, false);
        if ((style_ & EXPAND_BITS) == EXPAND) {
            cur_rect = parameter.rect;
        } else {
            cur_rect.x = parameter.rect.x;
            cur_rect.y = parameter.rect.y;
        }
        update_text_coord();

    } else if (evt == EVT_KEYBOARD && focus_) {
        mark_dirty(false, false);

        // no consideration on focus

        auto key    = parameter.pos.x;
        auto action = parameter.pos.y;
        if (action == GLFW_PRESS) {
            if (key == GLFW_KEY_RIGHT) {
                editor.on_right();
            } else if (key == GLFW_KEY_LEFT) {
                editor.on_left();
            } else if (key == GLFW_KEY_DELETE) {
                editor.on_delete_();
            } else if (key == GLFW_KEY_BACKSPACE) {
                editor.on_backspace();
            } else if (key == GLFW_KEY_ENTER) {
                event_at(EVT_FOCUS_OUT, at, parameter);
                return;
            }
        }

    } else if (evt == EVT_MOUSE_LEFT) {
        focus_      = true;
        auto action = parameter.pos.x;
        auto mods   = parameter.pos.y;
        if (mods == 0) {
            mark_dirty(false, false);
            if (action == GLFW_PRESS) {
                editor.on_mouse_down(eval_cursor_pos(at.x));
            } else {
                editor.on_mouse_up(eval_cursor_pos(at.x));
            }
        }

    } else if (evt == EVT_MOVE && focus_) {
        mark_dirty(false, false);
        editor.on_mouse_move(eval_cursor_pos(at.x));

    } else if (evt == EVT_CHAR && focus_) {
        if (wtext < cur_rect.w) {
            mark_dirty(false, false);
            editor.on_insert(parameter.i);
        }
    } else if (evt == EVT_FOCUS) {
        spdlog::info("TextCtrl::event_at(EVT_FOCUS)");
        mark_dirty(false, false);
        if (auto win = window_.lock()) {
            win->set_focus(shared_from_this());
        }
        focus_ = true;
    } else if (evt == EVT_FOCUS_OUT) {
        spdlog::info("TextCtrl::event_at(EVT_FOCUSOUT)");
        mark_dirty(false, false);
        focus_ = false;
    }
    WidgetBase::event_at(evt, at, parameter);
}

void TextCtrl::update_text_coord() {
    if ((style_ & mf::ALIGN_HORIZONTAL_BITS) == mf::ALIGN_CENTER &&
        (style_ & mf::ALIGN_VERTICAL_BITS) == mf::ALIGN_CENTER) {
        // wtext = std::min((GLuint)eval_text_width(),cur_rect.w);
        // htext = std::min(fontsize_,cur_rect.h);
        wtext = eval_text_width();
        htext = fontsize_;
        // xtext = std::max((int)cur_rect.x+((int)cur_rect.w-wtext)/2,0);
        // ytext = std::max((int)cur_rect.y+((int)cur_rect.h-htext)/2,0);
        ytext = (int)cur_rect.y + ((int)cur_rect.h - htext) / 2;
        xtext = (int)cur_rect.x + ((int)cur_rect.w - wtext) / 2;
    } else {
        spdlog::error("TextCtrl::update_text_coord: unimplemented");
    }
}
