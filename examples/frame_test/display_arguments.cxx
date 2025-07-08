#include "parameter_dict.hxx"

#include <stdint.h>

#include "debug_struct.hxx"

using glm::vec3;

int main() {
    auto window = std::make_shared<mf::Window>();

    auto pdict = std::make_shared<mf::ParameterDict>();

    for (int i = 0; i < 3; i++) {
        pdict->add({
            //
            {"stringvar1", "hello"},
            {"double", 0.654585},
            {"vec3", vec3(.1, .2, .3)},
            {"intvar2", (int64_t)10} //
        });
    }

    window->set_root(pdict);
    window->set_focus(nullptr);
    // pdict->event_at(mf::EVT_FOCUS_OUT, mf::Pos(), mf::Rect());

    window->mainloop();
}