#pragma once

#include "checkfail.hxx"
#include "shader_program.hxx"

#include <memory>
#include <optional>
#include <string>
#include <vector>

#ifndef __gl_h_
    #include <glad/glad.h>
#endif

class TextureImageData {
    public:
    TextureImageData(const char *image_path);
    ~TextureImageData();

    // readonly's
    inline auto width() { return width_; }
    inline auto height() { return height_; }
    inline auto data() { return data_; }

    private:
    int            width_;
    int            height_;
    unsigned char *data_;
};

struct TextureParameter {
    TextureParameter(
        GLenum wrap_s = GL_REPEAT, GLenum wrap_t = GL_REPEAT, GLenum max_filt = GL_LINEAR,
        GLenum min_filt = GL_LINEAR
    );
    GLenum wrap_s;
    GLenum wrap_t;
    GLenum max_filt;
    GLenum min_filt;
    void   BindParameter();
};

// default texture format: rgba-rgba
class TextureObject {
    public:
    TextureObject(
        std::string      name      = "", // internal name for texure
        GLuint           tex_index = 0,  // GL_TEXURE<x> also the value for uniform sampler2D
        TextureParameter parms     = TextureParameter(),
        GLenum           format    = GL_RGBA // assume format won't change throughout lifetime
        // default type is gl_texuture_2d
    );
    TextureObject(TextureObject &&o);
    TextureObject(const TextureObject &) = delete;
    ~TextureObject();

    void bind();
    void validate();
    void activate(GLuint at = -1);
    void activate_sampler(std::shared_ptr<ShaderProgram> prog, std::string name = "", int at = -1);
    // flush data when texture<x> is truncated
    void flush();
    void from_data(GLuint *data, int w, int h, GLenum value_type = GL_NONE);
    void from_file(std::string filename, bool gen_mipmap = true, bool save = true);

    // variables
    TextureParameter parms;

    // readonly's
    auto inline ID() { return ID_; }
    auto inline tex_index() { return tex_index_; }
    auto inline name() { return name_; }
    auto inline tex_internal_index() { return tex_index_; }

    private:
    GLuint      ID_;
    GLuint      tex_index_; // always valid
    std::string name_;
    GLenum      format_;

    std::shared_ptr<TextureImageData> data_;

    bool gen_mipmap_;
};
