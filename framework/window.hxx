#pragma once

#include "glfw_inst.hxx"

#include "widget.hxx" //recursive include

#include <memory>
#include <optional>

#ifndef __gl_h_
    #include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>

namespace mf {
    class WidgetBase;
    class Window : public std::enable_shared_from_this<Window> {
        public:
        Window(GLuint w = 800, GLuint h = 600, std::string title = "");
        ~Window();

        void bind();
        void on_resize(int w, int h);
        void on_resize();
        void draw();

        void set_title(std::string title);
        void set_root(std::shared_ptr<WidgetBase> root);
        void set_root_window();
        void set_focus(std::shared_ptr<WidgetBase> focus);

        void repr();

        // glfw wrapper
        bool should_close();
        void poll_events();
        void swap_buffers();
        void mainloop();

        // readonly's
        inline auto window() { return window_; }

        protected:
        GLuint      width_;
        GLuint      height_;
        std::string title_;

        std::shared_ptr<GlfwInst>   glfw_inst_;
        GLFWwindow                 *window_;
        std::shared_ptr<WidgetBase> root_;
        std::shared_ptr<WidgetBase> focus_;

        // managed screen output buffer, can enable downsampling for further use
        std::unique_ptr<DrawableFrame> fbo_;

        public:
        static Window *get_window_inst(GLFWwindow *window);
        static Pos     get_cursor_pos(GLFWwindow *window);
        static void    default_framebuffer_size_callback(GLFWwindow *window, int width, int height);
        static void
        default_key_callback(GLFWwindow *window, int key, int scancode, int action, int modes);
        static void default_scroll_callback(GLFWwindow *window, double x, double y);
        static void default_move_callback(GLFWwindow *window, double x, double y); // gen move event
        static void default_mouse_callback(GLFWwindow *window, int button, int action, int mods);
        static void default_char_callback(GLFWwindow *window, unsigned int code);
    };
} // namespace mf