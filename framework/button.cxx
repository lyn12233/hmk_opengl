#include "button.hxx"
#include "config.hxx"
#include "textedit.hxx"
#include "utils.hxx"
#include "widget.hxx"

using mf::Button;

Button::Button(std::string text, GLuint w, GLuint h, GLuint fontsize, mf::FLAGS style) :
    StaticText(text, w, h, fontsize, style) {
    colors = {DEFAULT_BTN_BKGD, DEFAULT_BTN_FRGD, DEFAULT_BTN_BKGD_INV, DEFAULT_BTN_FRGD_INV};
    mark_dirty();
}
Button::~Button() {}

void Button::event_at(EVENT evt, Pos at, EVENT_PARM parameter) {
    if (evt == EVT_RESIZE) {
        StaticText::event_at(evt, at, parameter);
        return;
    }
    // omit other events from StaticText::event_at
    if (evt == EVT_MOUSE_LEFT) {
        auto action = parameter.pos.x;
        auto mods   = parameter.pos.y;
        if (mods == 0) {
            if (action == GLFW_PRESS) {
                event_at(EVT_BUTTON, Pos(), EVENT_PARM());
            } else {
                event_at(EVT_BUTTON_UP, Pos(), EVENT_PARM());
            }
        }
    } else if (evt == EVT_BUTTON) {
        spdlog::debug("Button::event_at(button)");
        mark_dirty();
        colors = {DEFAULT_BTN_BKGD_INV, DEFAULT_BTN_FRGD_INV, DEFAULT_BTN_BKGD, DEFAULT_BTN_FRGD};
    } else if (evt == EVT_BUTTON_UP) {
        spdlog::debug("Button::event_at(button_up)");
        mark_dirty();
        colors = {DEFAULT_BTN_BKGD, DEFAULT_BTN_FRGD, DEFAULT_BTN_BKGD_INV, DEFAULT_BTN_FRGD_INV};
    }
    WidgetBase::event_at(evt, at, parameter);
}

// todo: add hover state for button