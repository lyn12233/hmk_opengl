#pragma once

#include <GLFW/glfw3.h>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>

#ifndef __gl_h_
    #include <glad/glad.h>
#endif

using glm::vec2;

namespace mf {

    // data structures

    struct Pos {
        int x;
        int y;
        inline Pos(int x = 0, int y = 0) : x(x), y(y) {}
    };
    struct Rect {
        int x;
        int y;
        int w;
        int h;
        inline Rect(int x = 0, int y = 0, int w = 0, int h = 0) : x(x), y(y), w(w), h(h) {}
        inline Pos  from() { return Pos(x, y); }
        inline Pos  to() { return Pos(x + w, y + h); }
        inline bool operator==(Rect r) { return x == r.x && y == r.y && w == r.w && h == r.h; }
        inline bool contains(Pos a) { return a.x >= x && a.x <= x + w && a.y >= y && a.y <= y + h; }
        inline bool contains(Rect r) { return contains(r.from()) && contains(r.to()); }
    };

    // event and parameter

    enum EVENT {
        // event           //parameter struct
        // common events
        EVT_RESIZE   = 0,
        EVT_LAYOUT   = 0,
        EVT_KEYBOARD = 1, // Pos(key,action)
        EVT_SCROLL   = 2, // vec2(xscrl,yscrl)
        EVT_ROUTINE  = 3, //()

        // mouse events
        EVT_MOUSE_LEFT   = 4, // Pos(action{PRESS|RELEASE},mods{CTRL|ALT|...})
        EVT_MOUSE_RIGHT  = 5,
        EVT_MOUSE_MIDDLE = 6,

        // move events
        EVT_MOVE = 8, //()

        // char
        EVT_CHAR = 9, // int(code)

        // focus
        EVT_FOCUS     = 10,
        EVT_FOCUS_OUT = 11,
    };
    union EVENT_PARM {
        vec2   vec2;
        Pos    pos;
        Rect   rect;
        int    i;
        float  as_float;
        double as_double;
        inline EVENT_PARM() : rect(Rect()) {}
        inline EVENT_PARM(Pos x) : pos(x) {}
        inline EVENT_PARM(Rect x) : rect(x) {}
        inline EVENT_PARM(int x) : i(x) {}
        inline EVENT_PARM(::vec2 x) : vec2(x) {}
    };
    
    typedef void EventHandler_t(EVENT evt, Pos pos, EVENT_PARM parameter);

    namespace key_utils {
        inline bool is_printable_key(int c) {
            return c >= GLFW_KEY_SPACE && c <= GLFW_KEY_GRAVE_ACCENT;
        }
        inline bool is_function_key(int c) { return c >= GLFW_KEY_ESCAPE && c <= GLFW_KEY_MENU; }
    } // namespace key_utils

    // window attributes

    enum WIDGET_TYPE { HSIZER, STATIC_TEXT, WORLD_VIEW, VSIZER };
    enum FLAGS : glm::u64 { // flag bits: 0-2: alignment;
        ALIGN_CENTER          = 0,
        ALIGN_BITS            = 15,
        ALIGN_RIGHT           = 1,
        ALIGN_LEFT            = 2,
        ALIGN_HORIZONTAL_BITS = 3, // 11
        ALIGN_TOP             = 1 << 2,
        ALIGN_BOTTOM          = 2 << 2,
        ALIGN_VERTICAL_BITS   = 12, // 1100
        EXPAND                = 1 << 4,
        EXPAND_BITS           = 1 << 4,
        // sizer
        SIZER_HORIZONTAL = 1 << 5,
        SIZER_VERTICAL   = 0,
        SIZER_BITS       = 1 << 5,
    };

} // namespace mf