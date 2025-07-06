#pragma once

#include "buffer_objects.hxx"
#include "drawable_frame.hxx"
#include "utils.hxx"

#include "window.hxx" // recursive include

#include <functional>
#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

#ifndef __gl_h_
    #include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>

#ifndef FRAME_DOWNSAMPLE_SCALE // shouldnt be here
    #define FRAME_DOWNSAMPLE_SCALE 2
#endif

#define ASSIGN_CASING(x, val)                                                                      \
    case val:                                                                                      \
        x = #val;                                                                                  \
        break;

using glm::vec2;

/*  concepts:
    pixel order is axis0:right-left axis1:top-bottom
    resize routine: window.resize-(pass resize event)->#sizer.resize(call layout)->...->
    dirtyness control:
        - draw may condition by dirty, for frequently updated frame, may not
        - dirtyness includes resized and moved, no distinction between
        - mark_dirty() may propagate upward or downward
        - mark_dirty() should solely called by event handlers
        - considering dirtyness contains double semantics(to simplify),
            drawcall may always propagate downward
        - for example, the frequently updated do not propagate dirtyness,
            but should receive a propagated draw call;
*/

namespace mf { // mini framework
    class Window;

    //*must use make_shared* polymorphic widget base, intended to list diiferent kinds of widgets
    class WidgetBase : public std::enable_shared_from_this<WidgetBase> {
        public:
        WidgetBase(
            GLuint w = 100, GLuint h = 100, std::weak_ptr<Window> window = {},
            FLAGS style = (FLAGS)0
        );
        virtual ~WidgetBase() = default;

        virtual void validate();               // default to check children set
        virtual bool draw(DrawableFrame &fbo); // default to control dirtyness
        virtual void event_at(EVENT evt, Pos at, EVENT_PARM parameter); // default to propagate
        virtual void calc_fixed_size(); // kind of flexible size control, no in consideration yet

        // relation
        std::vector<std::shared_ptr<WidgetBase>> children;
        std::weak_ptr<WidgetBase>                parent;
        std::weak_ptr<Window>                    window_;

        virtual void add_child(std::shared_ptr<WidgetBase> child);
        virtual void set_window(std::weak_ptr<Window> win);

        // states
        bool  dirty = true;
        void  mark_dirty(bool downward = true, bool upward = false);
        bool  focus_ = false;
        FLAGS style_;

        // slots
        std::vector<EVENT>                         handler_evt_tags;
        std::vector<std::function<EventHandler_t>> handlers;
        void bind_event(EVENT evt, std::function<EventHandler_t> handler);

        // info
        virtual void repr(int level = 0);

        // readonly's // not
        //  protected:
        WIDGET_TYPE type;
        GLuint      fixed_width_;
        GLuint      fixed_height_;
        Rect        cur_rect;
    };

}; // namespace mf