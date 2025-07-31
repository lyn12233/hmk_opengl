#include "button.hxx"
#include "utils.hxx"
#include "window.hxx"

#include "debug_struct.hxx"
#include <memory>

using namespace mf;
int main() {
    auto window = std::make_shared<mf::Window>();
    auto btn    = std::make_shared<mf::Button>("press me!");
    btn->bind_event(mf::EVT_BUTTON, [](EVENT evt, Pos pos, EVENT_PARM parm) {
        spdlog::debug("handling button event...");
    });
    window->set_root(btn);
    window->mainloop();
}