#include "widget.hxx"
#include "buffer_objects.hxx"
#include "utils.hxx"

#include <algorithm>
#include <iterator>
#include <memory>
#include <numeric>
#include <vector>

#include <spdlog/spdlog.h>

using mf::WidgetBase;

// constructor
WidgetBase::WidgetBase(GLuint w, GLuint h, std::weak_ptr<Window> window, FLAGS style) :
    fixed_width_(w), fixed_height_(h), cur_rect(0, 0, w, h), style_(style) {
    if (window.expired()) {
        // spdlog::warn("WidgetBase::WidgetBase: belongs to no window");
    }
    window_ = window;
}

// validate
void WidgetBase::validate() {
    for (auto &c : children) {
        if (!c) {
            spdlog::error("null widget in wdget tree");
            exit(-1);
        }
        // c->validate(); // do not walk through
    }
}
// draw

// return: if draw still needed for inherited classes
bool WidgetBase::draw(DrawableFrame &fbo) { return false; }

// default event_at: call dynamic handler and propagate
void WidgetBase::event_at(EVENT evt, Pos at, EVENT_PARM parameter) {

    // custom event handlers
    auto it = std::find(handler_evt_tags.begin(), handler_evt_tags.end(), evt);
    if (it != handler_evt_tags.end()) {
        auto handler = handlers[std::distance(handler_evt_tags.begin(), it)];
        handler(evt, at, parameter);
    }

    if (evt == EVT_ROUTINE) {
        // spdlog::debug("WidgetBase::event_at(ROUTINE)");
        for (auto &c : children)
            c->event_at(EVT_ROUTINE, Pos(), 0);
    } else if (evt == EVT_RESIZE) {
        cur_rect = parameter.rect;
    } else {
        for (auto &c : children) {
            if (c->cur_rect.contains(at)) {
                c->event_at(evt, at, parameter);
                break;
            }
        }
    }
}
void WidgetBase::calc_fixed_size() {} // assume fixed size won't change

// add_child
void WidgetBase::add_child(std::shared_ptr<WidgetBase> child) {
    spdlog::debug("WidgetBase::add_child(child={})", (bool)child);
    // may be called at construction, thus this is omitted
    // child->parent = shared_from_this(); // failed
    if (child) children.push_back(child);
}

void WidgetBase::set_child_parent() {
    for (const auto &c : children) {
        c->parent = shared_from_this();
        c->set_child_parent();
    }
}

// set parent
void WidgetBase::set_window(std::weak_ptr<Window> win) {
    window_ = win;
    for (auto &c : children) {
        c->set_window(win);
    }
}

// mark_dirty
void WidgetBase::mark_dirty(bool downward, bool upward) {
    dirty = true;
    // traceback
    if (upward) {
        if (auto p = parent.lock()) {
            if (p->dirty == false) {
                p->mark_dirty();
            }
        }
    }
    // propagate
    if (downward) {
        for (auto &c : children) {
            if (c->dirty == false) {
                c->mark_dirty();
            }
        }
    }
}
// bind
void WidgetBase::bind_event(EVENT evt, std::function<EventHandler_t> handler) {
    auto it = std::find(handler_evt_tags.begin(), handler_evt_tags.end(), evt);
    if (it != handler_evt_tags.end()) {
        handlers[std::distance(handler_evt_tags.begin(), it)] = handler;
    } else {
        handler_evt_tags.push_back(evt);
        handlers.push_back(handler);
    }
}

// represent
void WidgetBase::repr(int level) {
    std::string stype;
    switch (type) {
        ASSIGN_CASING(stype, HSIZER)
        ASSIGN_CASING(stype, VSIZER)
        ASSIGN_CASING(stype, STATIC_TEXT)
        ASSIGN_CASING(stype, WORLD_VIEW)
    }
    spdlog::info(
        std::string(level, ' ') + "({}, rect=({},{},{},{})){}", stype, cur_rect.x, cur_rect.y,
        cur_rect.w, cur_rect.h, dirty ? '#' : ' '
    );
    for (auto &c : children) {
        c->repr(level + 4);
    }
}
