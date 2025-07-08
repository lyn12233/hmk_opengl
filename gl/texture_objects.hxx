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

/// @addtogroup gl_wrappers
/// @{

namespace glwrapper {

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
            GLenum wrap_s = GL_CLAMP_TO_EDGE, GLenum wrap_t = GL_CLAMP_TO_EDGE,
            GLenum wrap_r = GL_CLAMP_TO_EDGE, GLenum max_filt = GL_LINEAR,
            GLenum min_filt = GL_LINEAR, GLenum type = GL_TEXTURE_2D
        );
        GLenum wrap_s;
        GLenum wrap_t;
        GLenum wrap_r;
        GLenum max_filt;
        GLenum min_filt;
        GLenum type;
        void   BindParameter();
    };

    // default texture format: rgba-rgba
    class TextureObject {

        public:
        //
        /// @brief constructor
        /// @brief wrapper for glGenTextures and glDeleteTextures
        /// @param name texture name that can be accessed like "uniform sampler.. <name>" in glsl,
        /// may be overriden
        /// @param tex_index GL_TEXURE<x> also the value for uniform sampler2D. may be overriden
        /// @param parms
        /// @param format texture color format, e.g. RBGA and DEPTH_COMPONENT
        /// @param type e.g. GL_TEXTURE_2D
        TextureObject(
            std::string      name      = "", // internal name for texure
            GLuint           tex_index = 0,  // GL_TEXURE<x> also the value for uniform sampler2D
            TextureParameter parms     = TextureParameter(),
            GLenum           format    = GL_RGBA, // assume format won't change throughout lifetime
            GLenum           type      = GL_TEXTURE_2D
        );

        TextureObject(TextureObject &&o);
        TextureObject(const TextureObject &) = delete;
        ~TextureObject();

        /// @brief wrapper for glBindTexture
        void bind();

        /// @brief validate tex_index, format and type of the texture
        void validate();

        /// @brief wrapper for glActiveTexture (+bind())
        /// @param at texture index, to override default tex_index
        void activate(GLuint at = -1);

        /// @brief activate + set shader_program uniform value
        void
        activate_sampler(std::shared_ptr<ShaderProgram> prog, std::string name = "", int at = -1);

        /// @brief flush data_(:texture_image_data) when texture<x> is truncated. unused(?)
        void flush();

        /// @brief wrapper for glTexImage2D. load data from pointer
        void from_data(
            void *data, int w, int h, GLenum value_type = GL_NONE, GLenum input_format = GL_NONE
        );
        /// @brief wrapper for glTexImage3D. load data from pointer
        void from_data(
            void *data, int w, int h, int d, GLenum value_type = GL_NONE,
            GLenum input_format = GL_NONE
        );
        /// @brief default input value type to UNSIGNED_BYTE(8UC4) for RGBA format or
        /// GL_FLOAT(32FC1) for DEPTH_COMPONENT format
        GLenum from_data_get_value_type(GLenum value_type);
        GLenum from_data_get_input_format(GLenum input_format);

        /// @brief read 2d texture from file
        /// @param filename
        /// @param gen_mipmap
        /// @param save save image data to data_
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
        GLenum      type_;

        std::shared_ptr<TextureImageData> data_;

        bool gen_mipmap_;
    };

} // namespace glwrapper

/// @}
// end of group