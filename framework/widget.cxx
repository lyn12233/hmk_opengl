#include "widget.hxx"
#include "buffer_objects.hxx"
#include "utils.hxx"

#include <algorithm>
#include <iterator>
#include <memory>
#include <numeric>
#include <vector>

#include <spdlog/spdlog.h>

using mf::HSizer;
using mf::WidgetBase;

// constructor
WidgetBase::WidgetBase(GLuint w, GLuint h, std::weak_ptr<Window> window, FLAGS style)
    : fixed_width_(w), fixed_height_(h), cur_rect(0, 0, w, h), style_(style) {
    if (window.expired()) {
        spdlog::warn("WidgetBase::WidgetBase: belongs to no window");
        // window = Window::get_window_inst(glfwGetCurrentContext())->shared_from_this();
        // spdlog::warn("here");
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
    auto it = std::find(handler_evt_tags.begin(), handler_evt_tags.end(), evt);
    if (it != handler_evt_tags.end()) {
        auto handler = handlers[std::distance(handler_evt_tags.begin(), it)];
        handler(evt, at, parameter);
    }

    if (evt == EVT_ROUTINE) {
        // spdlog::debug("WidgetBase::event_at(ROUTINE)");
        for (auto &c : children)
            c->event_at(EVT_ROUTINE, Pos(), 0);
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
    child->parent = shared_from_this(); // failed
    // spdlog::debug("WidgetBase::add_child: push child");
    children.push_back(child);
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

// horizontal sizer---

HSizer::HSizer(GLuint w, GLuint h, GLuint border) : WidgetBase(w, h) {
    fixed_width_  = w;
    fixed_height_ = h;
    border_       = border; // border unused
    cur_rect      = Rect(0, 0, w, h);
    type          = HSIZER;
}
HSizer::~HSizer() { spdlog::debug("HSizer::~HSizer"); }

// validate: validate proportions
void HSizer::validate() {
    if (children.size() != proportions.size() || children.size() != styles.size()) {
        spdlog::error("entry pair mismatch");
        exit(-1);
    }
    for (auto &i : proportions) {
        i = (i < 0.) ? 0. : i;
    }
} //::validate/>

// layout according to cur_rect
void HSizer::layout() { // gen acc,calc children rect,propagate resize event
    spdlog::debug("HSizer::layout");
    validate();

    auto len    = children.size();
    fixed_acc   = std::vector<GLuint>(len + 1, 0);
    portion_acc = std::vector<float>(len + 1, 0);

    for (int i = 1; i < len + 1; i++) {
        fixed_acc[i]   = fixed_acc[i - 1] + children[i - 1]->fixed_width_;
        portion_acc[i] = portion_acc[i - 1] + proportions[i - 1];
    }

    // calculate sizable spaces
    int space_delta = (int)cur_rect.w - (int)fixed_acc[len];
    space_delta     = (space_delta < 0) ? 0 : space_delta;
    // handle zero division
    // if no portion provided, scatters on the right
    float total_portion = (portion_acc[len] <= 0.) ? 1. : portion_acc[len];
    float per_portion   = space_delta / total_portion;

    // spdlog::debug("number of child:{}, space delta:{}, total
    // proportion:{}",len,space_delta,total_portion);

    auto at2 = Rect(cur_rect);
    for (int i = 0; i < len; i++) {
        at2.x = cur_rect.x + fixed_acc[i] + portion_acc[i] * per_portion;
        at2.w = children[i]->fixed_width_ + proportions[i] * per_portion;

        auto style = styles[i];
        if ((style & EXPAND_BITS) != EXPAND && at2.h >= children[i]->fixed_height_) {
            if ((style & ALIGN_BITS) == ALIGN_CENTER) {
                at2.y = at2.y + (at2.h - children[i]->fixed_height_) / 2;
            } else if ((style & ALIGN_BITS) == ALIGN_TOP) {
            } else if ((style & ALIGN_BITS) == ALIGN_BOTTOM) {
                at2.y = at2.y + at2.h - children[i]->fixed_height_;
            }
            at2.h = children[i]->fixed_height_;
        }

        // spdlog::debug("at2:{},{},{},{}",at2.x,at2.y,at2.w,at2.h);
        if (at2.x >= cur_rect.x + cur_rect.w) break;
        children[i]->event_at(EVT_LAYOUT, Pos(), at2);
    }
}

// draw
bool HSizer::draw(DrawableFrame &fbo) {
    dirty = false;
    validate();

    for (auto &c : children) {
        c->draw(fbo);
    }
    return false;
} //::draw/>

// event_at
void HSizer::event_at(EVENT evt, Pos pos, EVENT_PARM parameter) {
    // spdlog::debug("HSizer::event_at");
    if (evt == EVT_RESIZE) {
        if (!(parameter.rect == cur_rect)) {
            cur_rect = parameter.rect;
            mark_dirty();
            layout();
        }
        return;
    } else { // propagate event
        WidgetBase::event_at(evt, pos, parameter);
    }
} //::event_at/>

// add
void HSizer::add(std::shared_ptr<WidgetBase> child, float proportion, FLAGS style) {
    // spdlog::debug("HSizer::add(proportion={})",proportion);
    add_child(child);
    // spdlog::debug("HSizer::add: pushback info");
    proportions.push_back(proportion);
    styles.push_back(style);
}
