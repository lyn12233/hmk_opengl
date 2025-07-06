#include "mycube.hxx"

#include "sizer.hxx"
#include "textedit.hxx"
#include "utils.hxx"
#include "widget.hxx"
#include "window.hxx"

// #include "debug_struct.hxx"

#include <memory>
#include <spdlog/spdlog.h>

using mf::BoxSizer;
using mf::EVENT, mf::EVENT_PARM, mf::Pos;
using mf::Window;

// void simple_cb(EVENT evt,Pos pos,EVENT_PARM parameter){spdlog::info("call
// on:{}",(int)evt);exit(-2);}

class MyWindow : public Window {
    public:
    MyWindow() : Window() {
        auto vsizer = std::make_shared<BoxSizer>(0, 0, 0, mf::SIZER_VERTICAL);
        auto h1     = std::make_shared<BoxSizer>(0, 0, 0, mf::SIZER_HORIZONTAL);
        auto world  = std::make_shared<MyWorld>();
        auto t1     = std::make_shared<mf::TextCtrl>("a");
        auto t2     = std::make_shared<mf::TextCtrl>("b");
        vsizer->add(h1, 1);
        vsizer->add(world, 4);
        auto f = [t1](EVENT evt, Pos pos, EVENT_PARM parameter) {
            t1->repr();
            exit(-1);
        };
        h1->add(t1, 1);
        h1->add(t2, 2);
        t1->bind_event(mf::EVT_FOCUS, f);
        set_root(vsizer);
    }
};

int main() {
    auto window_inst = std::make_shared<MyWindow>();

    window_inst->mainloop();
}
