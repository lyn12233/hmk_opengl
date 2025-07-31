#include "texture_objects.hxx"
#include "checkfail.hxx"
#include "shader_program.hxx"

#include <cassert>
#include <iostream>
#include <memory>
#include <optional>

#include "stb_image.h"
#include <spdlog/spdlog.h>
#include <stddef.h>

using glwrapper::TextureImageData;
using glwrapper::TextureObject;
using glwrapper::TextureParameter;

// TextureImageData

TextureImageData::TextureImageData(std::string image_path) {
    spdlog::info("loading image:{}", image_path);
    int nb_channels; // unused. always set format to RGBA

    // flip y axis
    stbi_set_flip_vertically_on_load(true);

    data_ = stbi_load(image_path.c_str(), &width_, &height_, &nb_channels, 4);
    if (!data_) {
        spdlog::error("cant load image: {}", image_path);
        exit(-1);
    }

    spdlog::info("loaded image {}(width={},height={})", image_path, width_, height_);
}

TextureImageData::TextureImageData(void *raw_image, size_t size) {
    spdlog::info("loading from raw image");
    int nb_channels;

    stbi_set_flip_vertically_on_load(true);
    data_ =
        stbi_load_from_memory((unsigned char *)raw_image, size, &width_, &height_, &nb_channels, 4);
    if (!data_) {
        spdlog::error("cant load image(from raw)");
        exit(-1);
    }

    spdlog::info("loaded image(width={},height={})", width_, height_);
}

TextureImageData::~TextureImageData() {
    spdlog::info("freeing image ({},{})", width_, height_);
    stbi_image_free(data_);
}

// TextureObject

TextureObject::TextureObject(
    std::string name, GLuint tex_index, TextureParameter parms, GLenum format, GLenum type,
    bool gen_mipmap
) :
    name_(name),
    tex_index_(tex_index),        //
    parms(parms),                 //
    format_(format), type_(type), //
    gen_mipmap_(gen_mipmap) {

    this->parms.type = type;

    validate();
    glGenTextures(1, &ID_);
}

TextureObject::TextureObject(TextureObject &&o) :
    ID_(o.ID_), tex_index_(o.tex_index_), name_(o.name_), data_(o.data_), format_(o.format_),
    gen_mipmap_(o.gen_mipmap_) {
    o.ID_ = 0;
}

TextureObject::~TextureObject() {
    if (ID_ == 0) return;
    spdlog::debug("TextureObject::~TextureObject(id={})", ID_);
    glDeleteTextures(1, &ID_);
}

void TextureObject::bind() { glBindTexture(type_, ID_); }

void TextureObject::validate() {
    if (tex_index_ >= 32) {
        spdlog::error("invalid texture index:{}", tex_index_);
        exit(-1);
    }
    if (format_map.count(format_) == 0) {
        spdlog::error("invalid texture format:{}", (int)format_);
        exit(-1);
    }
    if (type_ != GL_TEXTURE_2D && type_ != GL_TEXTURE_3D) {
        spdlog::error("invalid texture type: {}", (int)type_);
        exit(-1);
    }
}

void TextureObject::activate(GLuint at) {
    // decide tex_index
    if (at != -1) tex_index_ = at;
    validate();
    // activate and bind
    //  NOTE: activetexture must prior to the first bindtexture call
    glActiveTexture(GL_TEXTURE0 + tex_index_);
    bind();
}
void TextureObject::activate_sampler(
    std::shared_ptr<ShaderProgram> prog, std::string name, int at
) {
    MY_CHECK_FAIL
    prog->use();
    activate(at);
    name_ = (name == "" ? name_ : name);
    prog->set_value(name_, tex_index_);
}

void TextureObject::from_data(
    void *data, int width, int height, GLenum value_type, GLenum input_format
) {
    MY_CHECK_FAIL
    assert(type_ == GL_TEXTURE_2D);
    value_type   = from_data_parse_value_type(value_type);
    input_format = from_data_parse_input_format(input_format);

    // bind context
    bind();
    parms.BindParameter();

    MY_CHECK_FAIL

    glTexImage2D(
        GL_TEXTURE_2D, // tex type
        0,             // level of detail
        format_,       // internal format
        width, height,
        0,            // border MBZ
        input_format, // input format
        value_type,   // input type
        data          // input
    );

    MY_CHECK_FAIL

    if (gen_mipmap_) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    MY_CHECK_FAIL
}

void TextureObject::from_data(
    void *data, int width, int height, int depth, GLenum value_type, GLenum input_format
) {
    MY_CHECK_FAIL
    assert(type_ == GL_TEXTURE_3D);
    value_type   = from_data_parse_value_type(value_type);
    input_format = from_data_parse_input_format(input_format);

    // bind context
    bind();
    parms.BindParameter();

    MY_CHECK_FAIL

    glTexImage3D(
        GL_TEXTURE_3D, // tex type
        0,             // level of detail
        format_,       // internal format
        width, height, depth,
        0,            // border MBZ
        input_format, // input format
        value_type,   // input type
        data          // input
    );
    MY_CHECK_FAIL
    if (gen_mipmap_) {
        glGenerateMipmap(GL_TEXTURE_3D);
    }
    MY_CHECK_FAIL
}

void TextureObject::from_image(std::string filename, bool save) {
    auto img = std::make_shared<TextureImageData>(filename);
    from_data((void *)img->data(), img->width(), img->height());
    if (save) {
        spdlog::info("caching image");
        data_ = img;
    }
}
void TextureObject::from_image(void *raw_image, size_t size, bool save) {
    auto img = std::make_shared<TextureImageData>(raw_image, size);
    from_data((void *)img->data(), img->width(), img->height());
    if (save) {
        spdlog::info("caching image");
        data_ = img;
    }
}

void TextureObject::repr(int nb_component) {
    bind();
    int width, height;
    glGetTexLevelParameteriv(type_, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(type_, 0, GL_TEXTURE_HEIGHT, &height);
    auto tex_data = std::vector<float>(width * height * nb_component);
    glGetTexImage(type_, 0, format_map[format_].format, GL_FLOAT, tex_data.data());
    std::string log = fmt::format("{}x{}[", width, height);

    int i = 0;
    for (auto f : tex_data) {
        log += fmt::format("{:.3f} ", f);
        i++;
        if (i % nb_component == 0) {
            log += ";";
        }
        if (i % (width * nb_component) == 0) {
            log += "\n";
        }
    }
    spdlog::debug(log + "]");
}

std::map<GLenum, TextureObject::f_v> TextureObject::format_map = {
    {GL_R8, {GL_RED, GL_UNSIGNED_BYTE}},
    {GL_R32F, {GL_RED, GL_FLOAT}},
    {GL_RGB8, {GL_RGB, GL_UNSIGNED_BYTE}},
    {GL_RGBA8, {GL_RGBA, GL_UNSIGNED_BYTE}},
    {GL_RGB32F, {GL_RGB, GL_FLOAT}},
    {GL_RGBA32F, {GL_RGBA, GL_FLOAT}},
    {GL_DEPTH_COMPONENT24, {GL_DEPTH_COMPONENT, GL_UNSIGNED_INT}},
    {GL_DEPTH_COMPONENT32F, {GL_DEPTH_COMPONENT, GL_FLOAT}},
};

// TextureParameter

TextureParameter::TextureParameter(
    GLenum wrap_s, GLenum wrap_t, GLenum wrap_r, GLenum max_filt, GLenum min_filt, GLenum type
) :
    wrap_s(wrap_s),
    wrap_t(wrap_t), wrap_r(wrap_r), max_filt(max_filt), min_filt(min_filt), type(type) {}

TextureParameter::TextureParameter(std::string type) {
    if (type == "discrete") {
        *this = ::TextureParameter(
            GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST,
            GL_TEXTURE_2D
        );
    } else if (type == "smooth") {
        *this = ::TextureParameter(
            GL_REPEAT, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR, GL_TEXTURE_2D
        );
    } else {
        // *this = ::TextureParameter();
        spdlog::error("TextureParameter::TextureParameter: unknown builtin type: {}", type);
        exit(-1);
    }
}

void TextureParameter::BindParameter() {
    MY_CHECK_FAIL
    if (type == GL_TEXTURE_2D || type == GL_TEXTURE_3D) {
        glTexParameteri(type, GL_TEXTURE_WRAP_S, wrap_s);
        MY_CHECK_FAIL
        glTexParameteri(type, GL_TEXTURE_WRAP_T, wrap_t);
        MY_CHECK_FAIL
        glTexParameteri(type, GL_TEXTURE_MIN_FILTER, min_filt);
        MY_CHECK_FAIL
        glTexParameteri(type, GL_TEXTURE_MAG_FILTER, max_filt);
        MY_CHECK_FAIL
        if (type == GL_TEXTURE_3D) {
            glTexParameteri(type, GL_TEXTURE_WRAP_R, wrap_r);
            MY_CHECK_FAIL
        }
    } else {
        spdlog::debug("TextureParameter::BindParameter: unkwon texture type: {}", (int)type);
        exit(-1);
    }
    MY_CHECK_FAIL
}
