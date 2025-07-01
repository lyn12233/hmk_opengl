#include "texture_objects.hxx"
#include "checkfail.hxx"
#include "shader_program.hxx"

#include <iostream>
#include <memory>
#include <optional>

#include "stb_image.h"
#include <spdlog/spdlog.h>

// TextureImageData

TextureImageData::TextureImageData(const char *image_path) {
    spdlog::info("loading image:{}", image_path);
    int nb_channels; // unused. always set format to RGBA

    // flip y axis
    stbi_set_flip_vertically_on_load(true);

    data_ = stbi_load(image_path, &width_, &height_, &nb_channels, 4);
    if (!data_) {
        spdlog::error("cant load image: {}", image_path);
        exit(-1);
    }

    spdlog::info("loaded image {}(width={},height={},head=...", image_path, width_, height_);
}

TextureImageData::~TextureImageData() {
    spdlog::info("freeing image ({},{})", width_, height_);
    stbi_image_free(data_);
}

// TextureObject

TextureObject::TextureObject(
    std::string name, GLuint tex_index, TextureParameter parms, GLenum format
)
    : name_(name), tex_index_(tex_index), parms(parms), format_(format) {
    validate();
    glGenTextures(1, &ID_);
}

TextureObject::TextureObject(TextureObject &&o)
    : ID_(o.ID_), tex_index_(o.tex_index_), name_(o.name_), data_(o.data_), format_(o.format_),
      gen_mipmap_(o.gen_mipmap_) {
    o.ID_ = 0;
}

TextureObject::~TextureObject() {
    if (ID_ == 0) return;
    spdlog::debug("deleting TextureObject (id={})", ID_);
    glDeleteTextures(1, &ID_);
}

void TextureObject::bind() { glBindTexture(GL_TEXTURE_2D, ID_); }

void TextureObject::validate() {
    if (tex_index_ >= 32) {
        spdlog::error("invalid texture index:{}", tex_index_);
        exit(-1);
    }
    if (format_ != GL_RGBA && format_ != GL_DEPTH_COMPONENT) {
        spdlog::error("invalid texture format:{}", (int)format_);
        exit(-1);
    }
}

void TextureObject::activate(GLuint at) {
    // decide tex_index
    tex_index_ = (at == -1 ? tex_index_ : 0);
    validate();
    // activate and bind
    //  NOTE: activetexture must prior to bindtexture
    glActiveTexture(GL_TEXTURE0 + tex_index_);
    bind();
}
void TextureObject::activate_sampler(
    std::shared_ptr<ShaderProgram> prog, std::string name, int at
) {
    prog->use();
    activate(at);
    name_ = (name == "" ? name_ : name);
    prog->set_value(name_.c_str(), tex_index_);
}
void TextureObject::flush() {
    if (data_) { // shared_ptr data_ managed
        from_data((GLuint *)data_->data(), data_->width(), data_->height());
    } else {
        spdlog::warn("TextureObject::flush: warn: texture data not available");
    }
}
void TextureObject::from_data(GLuint *data, int width, int height, GLenum value_type) {
    if (value_type == GL_NONE) {
        if (format_ == GL_RGBA)
            value_type = GL_UNSIGNED_BYTE; // default 8UC4
        else if (format_ == GL_DEPTH_COMPONENT)
            value_type = GL_FLOAT; // default 32FC1
    }
    // bind context
    bind();
    parms.BindParameter();

    glTexImage2D(
        GL_TEXTURE_2D, // tex type
        0,             // level of detail
        format_,       // internal format
        width, height,
        0,          // border MBZ
        format_,    // input format
        value_type, // input type
        data        // input
    );
    if (gen_mipmap_) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
}
void TextureObject::from_file(
    std::string filename, bool gen_mipmap, bool save
) { // todo: consider clipping
    gen_mipmap_ = gen_mipmap;
    auto img    = std::make_shared<TextureImageData>(filename.c_str());
    from_data((GLuint *)img->data(), img->width(), img->height());
    if (save) {
        spdlog::info("caching image");
        data_ = img;
    }
}

// TextureParameter

TextureParameter::TextureParameter(GLenum wrap_s, GLenum wrap_t, GLenum max_filt, GLenum min_filt)
    : wrap_s(wrap_s), wrap_t(wrap_t), max_filt(max_filt), min_filt(min_filt) {}

void TextureParameter::BindParameter() {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filt);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, max_filt);
}
