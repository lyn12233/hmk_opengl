#include "debug_struct.hxx"

#include "sizer.hxx"
#include "textedit.hxx"
#include "utils.hxx"
#include "window.hxx"

#include <memory>

#include <spdlog/spdlog.h>

int main() {
    auto window_inst = std::make_shared<mf::Window>();
    {
        auto sizer =
            std::make_shared<mf::BoxSizer>(0, 0, 10, (mf::FLAGS)(mf::SIZER_VERTICAL | mf::EXPAND));
        auto tc = std::make_shared<mf::TextCtrl>("helloworld");
        auto st = std::make_shared<mf::StaticText>("hello:");
        sizer->add(tc, 1);
        sizer->add(st, 1);
        window_inst->set_root(sizer);
        window_inst->mainloop();
    }
}