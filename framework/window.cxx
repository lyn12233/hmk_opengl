#include "window.hxx"
#include "buffer_objects.hxx"
#include "checkfail.hxx"
#include "glfw_inst.hxx"
#include "utils.hxx"
#include "widget.hxx"

#include <cstddef>
#include <memory>

#include <spdlog/spdlog.h>

using mf::WidgetBase;
using mf::Window;

// window
Window::Window(GLuint w, GLuint h, std::string title) : width_(w), height_(h), title_(title) {
    spdlog::debug("Window::Window");
    glfwInit();
    // window
    window_ = glfwCreateWindow(width_, height_, title_.c_str(), NULL, NULL);
    if (!window_) {
        spdlog::error("failed to create window");
        exit(-1);
    }

    // make context current
    bind();

    glfw_inst_ = std::make_shared<GlfwInst>();
    glfw_inst_->load_proc();

    // set callbacks
    glfwSetWindowUserPointer(window_, (void *)this);
    glfwSetFramebufferSizeCallback(window_, default_framebuffer_size_callback);
    glfwSetKeyCallback(window_, default_key_callback);
    glfwSetScrollCallback(window_, default_scroll_callback);
    glfwSetCursorPosCallback(window_, default_move_callback);
    glfwSetMouseButtonCallback(window_, default_mouse_callback);
    glfwSetCharCallback(window_, default_char_callback);

    // fbo
    fbo_ = std::make_unique<DrawableFrame>(width_, height_);
    MY_CHECK_FAIL
}
Window::~Window() {
    MY_CHECK_FAIL
    spdlog::info("destroying window(Window::~Window)");
    // callback destroyed first
    glfwSetWindowUserPointer(window_, nullptr);
    MY_CHECK_FAIL
    fbo_.reset();
    MY_CHECK_FAIL
    // widget tree is destroyed second
    if (root_) {
        spdlog::debug("resetting root_(refs={})", root_.use_count());
        root_.reset(); // decrease refcount and set root_ nullptr
        MY_CHECK_FAIL
        spdlog::debug("root_ reset");
    }
    // glfwMakeContextCurrent(nullptr);//failed. gl context not completely released
    // MY_CHECK_FAIL
    // glfwDestroyWindow(window_);
    // MY_CHECK_FAIL
}

// bind
void Window::bind() { glfwMakeContextCurrent(window_); }
// resize
void Window::on_resize(int w, int h) {
    spdlog::debug("Window::on_resize");
    width_  = w;
    height_ = h;
    fbo_->set_cur_rect(mf::Rect(0, 0, w, h));
    if (root_) {
        root_->event_at(EVT_RESIZE, Pos(), Rect(0, 0, width_, height_));
    } else {
        spdlog::warn("no root found");
    }
    // draw(); draw should be called directly
}
void Window::on_resize() {
    int w, h;
    glfwGetWindowSize(window_, &w, &h);
    on_resize(w, h);
}
// draw
void Window::draw() {
    // spdlog::debug("Window::draw");
    MY_CHECK_FAIL
    if (root_) {
        // spdlog::debug("drawing root_...");
        fbo_->bind();
        MY_CHECK_FAIL
        root_->draw(*fbo_);
        MY_CHECK_FAIL
        fbo_->unbind();
    }
    glViewport(0, 0, width_, height_);
    fbo_->draw();
    MY_CHECK_FAIL

#ifdef _DEBUG
// repr();
#endif
}
// title
void Window::set_title(std::string title) {
    title_ = title;
    glfwSetWindowTitle(window_, title_.c_str());
}
// root
void Window::set_root(std::shared_ptr<WidgetBase> root) {
    spdlog::debug("Window::set_root");
    root_ = root;
    on_resize();
}
void Window::set_root_window() {
    if (root_) {
        root_->set_window(shared_from_this());
    }
}
// focus
void Window::set_focus(std::shared_ptr<WidgetBase> focus) {
    spdlog::debug("Window::set_focus (focus exists: {})", (bool)focus);
    if (focus != focus_) {
        if (focus_) {
            spdlog::debug("Window::set_focus: gen FOCUSOUT event");
            focus_->event_at(EVT_FOCUS_OUT, Pos(), EVENT_PARM());
        }
        focus_ = focus;
    }
}

// repr
void Window::repr() { root_->repr(); }

// wrappers

bool Window::should_close() { return glfwWindowShouldClose(window_); }
void Window::poll_events() { return glfwPollEvents(); }
void Window::swap_buffers() { glfwSwapBuffers(window_); }
void Window::mainloop() {
    set_root_window();
    while (!should_close()) {
        poll_events();
        if (root_) {
            root_->event_at(EVT_ROUTINE, Pos(), 0);
        }
        // clear left for child
        draw();
        swap_buffers();
    }
}

// static callback
Window *Window::get_window_inst(GLFWwindow *window) {
    return static_cast<Window *>(glfwGetWindowUserPointer(window));
}
mf::Pos Window::get_cursor_pos(GLFWwindow *window) {
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    return Pos((GLuint)x, (GLuint)y);
}
void Window::default_framebuffer_size_callback(GLFWwindow *window, int w, int h) {
    auto window_inst = get_window_inst(window);
    if (window_inst) {
        window_inst->on_resize(w, h);
        // #ifdef _DEBUG
        // spdlog::info("Framebuffer resized to {}x{}", w, h);
        // window_inst->repr();
        // #endif
    }
}

void Window::default_key_callback(
    GLFWwindow *window, int key, int scancode, int action, int modes
) {
    auto window_inst = get_window_inst(window);
    spdlog::debug(
        "Window::default_key_callback (focus exists:{})", (bool)(window_inst && window_inst->focus_)
    );
    if (window_inst && window_inst->focus_) {
        window_inst->focus_->event_at(EVT_KEYBOARD, get_cursor_pos(window), Pos(key, action));
    }
}

void Window::default_scroll_callback(GLFWwindow *window, double x, double y) {
    auto window_inst = get_window_inst(window);
#ifdef _DEBUG
    // spdlog::info("scroll at: {},{}",x,y);
    window_inst->repr();
#endif
    // the scroll is often y=\pm 1.
    if (window_inst && window_inst->root_) {
        window_inst->root_->event_at(EVT_SCROLL, get_cursor_pos(window), vec2(x, y));
    }
}

void Window::default_move_callback(GLFWwindow *window, double x, double y) {
    auto window_inst = get_window_inst(window);
    if (window_inst && window_inst->root_) {
        window_inst->root_->event_at(EVT_MOVE, Pos(x, y), Pos());
    }
}

void Window::default_mouse_callback(GLFWwindow *window, int button, int action, int mods) {
    auto window_inst = get_window_inst(window);
    if (window_inst && window_inst->root_) {
        mf::EVENT evt = (button == GLFW_MOUSE_BUTTON_LEFT)    ? EVT_MOUSE_LEFT
                        : (button == GLFW_MOUSE_BUTTON_RIGHT) ? EVT_MOUSE_RIGHT
                                                              : EVT_MOUSE_MIDDLE;
        window_inst->root_->event_at(evt, get_cursor_pos(window), Pos(action, mods));
        if (action == GLFW_PRESS) {
            spdlog::info("focus: ({},{})", get_cursor_pos(window).x, get_cursor_pos(window).y);
            window_inst->root_->event_at(EVT_FOCUS, get_cursor_pos(window), Rect());
        }
    }
}

void Window::default_char_callback(GLFWwindow *window, unsigned int code) {
    auto window_inst = get_window_inst(window);
    if (window_inst && window_inst->focus_) {
        window_inst->focus_->event_at(EVT_CHAR, get_cursor_pos(window), code);
    }
}