#include "debug_struct.hxx"

#include "textedit.hxx"
#include "window.hxx"

#include <memory>

#include <spdlog/spdlog.h>

int main(){
    auto window_inst=std::make_shared<mf::Window>();
    {
        auto tc=std::make_shared<mf::TextCtrl>("helloworld");
        window_inst->set_root(tc);
        window_inst->mainloop();
    }

}