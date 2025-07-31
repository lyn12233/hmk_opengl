#pragma once

#include "textedit.hxx"

namespace mf {
    class Button : public StaticText {
        public:
        Button(
            std::string text = "", GLuint w = DEFAULT_TC_WIDTH, GLuint h = DEFAULT_TC_HEIGHT,
            GLuint    fontsize = DEFAULT_TC_FONTSIZE,
            mf::FLAGS style    = static_cast<FLAGS>(mf::ALIGN_CENTER | mf::EXPAND)
        );
        ~Button() override;
        void event_at(EVENT evt, Pos at, EVENT_PARM parameter) override;
    };
} // namespace mf