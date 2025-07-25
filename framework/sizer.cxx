#include "sizer.hxx"

#include "drawable_frame.hxx"
#include "utils.hxx"
#include "widget.hxx"

#include <spdlog/spdlog.h>

using mf::BoxSizer;

BoxSizer::BoxSizer(
    GLuint w, GLuint h,
    GLuint border, // 2n+1 borders
    FLAGS  style
) :
    WidgetBase(w, h, {}, style) {
    fixed_width_  = w;
    fixed_height_ = h;
    border_       = border; // border unused
    cur_rect      = Rect(0, 0, 0, 0);
    type          = (style & SIZER_BITS) == SIZER_HORIZONTAL ? HSIZER : VSIZER;
}

BoxSizer::~BoxSizer() { spdlog::debug("Sizer::~Sizer"); }

void BoxSizer::validate() {
    if (children.size() != proportions.size() || children.size() != styles.size()) {
        spdlog::error("entry pair mismatch");
        exit(-1);
    }
    for (auto &i : proportions) {
        i = (i < 0.) ? 0. : i;
    }
} //::validate/>

void BoxSizer::layout() {
    // spdlog::debug("Sizer::layout (num_child:{})", children.size());
    validate();

    auto len    = children.size();
    fixed_acc   = std::vector<GLuint>(len + 1, 0); // fixed accumulation
    portion_acc = std::vector<float>(len + 1, 0);  // portion accumulation

    fixed_acc[0] = border_;
    for (int i = 1; i < len + 1; i++) {
        fixed_acc[i] = fixed_acc[i - 1];
        fixed_acc[i] +=
            ((type == HSIZER) ? children[i - 1]->fixed_width_ : children[i - 1]->fixed_height_);
        fixed_acc[i] += border_;
        portion_acc[i] = portion_acc[i - 1] + proportions[i - 1];
    }

    // calculate resizeable spaces
    int space_delta = (type == HSIZER) ? cur_rect.w : cur_rect.h;
    space_delta     = std::max(space_delta - (int)fixed_acc[len], 0);
    // portion unit
    float total_portion = (portion_acc[len] <= 0.) ? 1. : portion_acc[len];
    float per_portion   = space_delta / total_portion;

    // iterate child
    for (int i = 0; i < len; i++) {
        auto  r2       = cur_rect;
        auto &r2_x     = (type == HSIZER) ? r2.x : r2.y;
        auto &r2_y     = (type == HSIZER) ? r2.y : r2.x;
        auto &r2_w     = (type == HSIZER) ? r2.w : r2.h;
        auto &r2_h     = (type == HSIZER) ? r2.h : r2.w;
        auto  child_fw = (type == HSIZER) ? children[i]->fixed_width_ : children[i]->fixed_height_;
        auto  child_fh = (type == HSIZER) ? children[i]->fixed_height_ : children[i]->fixed_width_;

        // manage <horizontal> part
        if (fixed_acc[i] + portion_acc[i] * per_portion > r2_w) break;
        r2_x += fixed_acc[i] + portion_acc[i] * per_portion;
        r2_w = child_fw + proportions[i] * per_portion;

        // manage <height> expansion
        if ((styles[i] & EXPAND_BITS) != EXPAND && r2_h > child_fh) {
            if ((styles[i] & ALIGN_BITS) == ALIGN_CENTER) {
                r2_y += (r2_h - child_fh) / 2;
            } else if ((styles[i] & ALIGN_BITS) == ALIGN_TOP) {
                // nop;
            } else if ((styles[i] & ALIGN_BITS) == ALIGN_BOTTOM) {
                r2_y += r2_h - child_fh;
            }
            r2_h = child_fh;
        }

        children[i]->event_at(EVT_LAYOUT, Pos(), r2);
    }
}

// draw
bool BoxSizer::draw(mf::DrawableFrame &fbo) {
    dirty = false;
    validate();

    for (auto &c : children) {
        c->draw(fbo);
    }
    return false;
} //::draw/>

// event_at
void BoxSizer::event_at(EVENT evt, Pos pos, EVENT_PARM parameter) {
    if (evt == EVT_RESIZE) {
        if (!(parameter.rect == cur_rect)) {
            cur_rect = parameter.rect;
            mark_dirty();
            layout();
        }
        return;
    } else { // propagate event. this means dynamic resize callback won't be called
        WidgetBase::event_at(evt, pos, parameter);
    }
} //::event_at/>

// add
void BoxSizer::add(std::shared_ptr<WidgetBase> child, float proportion, FLAGS style) {
    spdlog::debug("Sizer::add(proportion={}, style={})", proportion, (int)style);
    add_child(child);
    proportions.push_back(proportion);
    styles.push_back(style);
}