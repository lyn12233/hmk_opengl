#pragma once

#include "buffer_objects.hxx"
#include "config.hxx"
#include "drawable_frame.hxx"
#include "utils.hxx"
#include "widget.hxx"


namespace mf {
    class BoxSizer : public WidgetBase {
        public:
        BoxSizer(
            GLuint w      = 0,                    // fixed width
            GLuint h      = 0,                    // fixed height
            GLuint border = DEFAULT_SIZER_BORDER, // 2n+1 borders
            FLAGS  style  = SIZER_HORIZONTAL
        );
        ~BoxSizer() override;

        void validate() override;
        void layout();
        bool draw(DrawableFrame &fbo) override;
        void event_at(EVENT evt, Pos at, EVENT_PARM parameter) override;

        void
        add(std::shared_ptr<WidgetBase> child, float proportion = 0.,
            FLAGS style = static_cast<FLAGS>(EXPAND | ALIGN_CENTER));

        protected:
        GLuint             border_;
        std::vector<float> proportions;
        std::vector<FLAGS> styles;

        std::vector<GLuint> fixed_acc;
        std::vector<float>  portion_acc;
        std::vector<Rect>   rects;
    };
} // namespace mf