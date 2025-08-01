#define _CRT_SECURE_NO_WARNINGS

#include "parameter_dict.hxx"
#include "sizer.hxx"
#include "textedit.hxx"
#include "utils.hxx"

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <stdint.h>
#include <string>
#include <variant>

#include <spdlog/spdlog.h>

using mf::ParameterDict;
using mf::ParameterTC;

ParameterDict::ParameterDict(int entry_per_col) :
    BoxSizer(0, 0, 0, SIZER_HORIZONTAL), entry_per_col_(entry_per_col) {}

ParameterDict::ParameterDict(ParameterDict_t dict, int entry_per_col) :
    BoxSizer(0, 0, 0, SIZER_HORIZONTAL), entry_per_col_(entry_per_col) {
    add(dict);
}

ParameterDict::~ParameterDict() {}

void ParameterDict::add(string name, ParameterDictVar_t value) { //
    spdlog::debug("ParameterDict::add: add entry({})", name);

    auto vsizer  = acquire_line_();
    auto hsizer  = std::make_shared<BoxSizer>(0, 30, 0, mf::SIZER_HORIZONTAL);
    auto text    = std::make_shared<StaticText>(name);
    auto var_str = std::make_shared<ParameterTC>(
        default_var_validator(value) ? default_var_formatter(value) : "", (PDTYPE)(value.index())
    );

    hsizer->add(text, 1.);
    hsizer->add(var_str, 2.);
    vsizer->add(hsizer, 0.);

    auto callback = [this, text, var_str](EVENT, Pos, EVENT_PARM) { //
        this->content_changed_ = true;

        auto &var     = this->data_[text->get_text()];
        auto  str     = var_str->get_text();
        auto  new_str = default_str_evaluator(var, str);

        spdlog::trace("var set to {}", default_var_formatter(this->data_[text->get_text()]));

        var_str->set_text(new_str);
    };
    var_str->bind_event(EVT_FOCUS_OUT, callback);

    data_[name] = value;
}

void ParameterDict::add(ParameterDict_t many) {
    for (auto [name, value] : many) {
        add(name, value);
    }
}

std::shared_ptr<mf::BoxSizer> ParameterDict::acquire_line_() {
    if (children.size() == 0 || children[children.size() - 1]->children.size() >= entry_per_col_) {
        auto vsizer = std::make_shared<BoxSizer>(0, 0, 10, mf::SIZER_VERTICAL);
        BoxSizer::add(vsizer, 1.);
        return vsizer;
    } else {
        return std::static_pointer_cast<BoxSizer>(children[children.size() - 1]);
    }
}

std::string ParameterDict::default_var_formatter(const ParameterDictVar_t &var) {
    if (std::holds_alternative<int64_t>(var)) {
        return fmt::to_string(std::get<int64_t>(var));
    } else if (std::holds_alternative<double>(var)) {
        return fmt::format("{:.4f}", std::get<double>(var));
    } else if (std::holds_alternative<string>(var)) {
        return std::get<string>(var);
    } else if (std::holds_alternative<vec3>(var)) {
        auto v = std::get<vec3>(var);
        return fmt::format("{:.3f},{:.3f},{:.3f}", v.x, v.y, v.z);
    }
    return "??";
}
bool ParameterDict::default_var_validator(const ParameterDictVar_t &var) { return true; }

std::string ParameterDict::default_str_evaluator(ParameterDictVar_t &var, string text) {
    if (std::holds_alternative<int64_t>(var)) {
        var = (int64_t)std::atoll(text.c_str());
    } else if (std::holds_alternative<double>(var)) {
        var = (double)std::atof(text.c_str());
    } else if (std::holds_alternative<string>(var)) {
        var = text;
    } else if (std::holds_alternative<vec3>(var)) {
        float x = 0, y = 0, z = 0;
        std::sscanf(text.c_str(), "%f,%f,%f", &x, &y, &z);
        var = vec3(x, y, z);
    } else {
        return "??";
    }
    return default_var_formatter(var);
}

ParameterTC::ParameterTC(
    std::string text, PDTYPE type, GLuint w, GLuint h, GLuint fontsize, mf::FLAGS style
) :
    TextCtrl(text, w, h, fontsize, style),
    pd_type_(type) {}

ParameterTC::~ParameterTC() {}

void ParameterTC::event_at(EVENT evt, Pos at, EVENT_PARM parameter) {
    if (evt == EVT_FOCUS) {
        editor.on_select_all();
    } else if (evt == EVT_SCROLL && focus_ && pd_type_ == PD_DOUBLE) {
        spdlog::debug("ParameterTC::event_at (scroll)");
        auto y = parameter.vec2.y;

        ParameterDictVar_t var = 0.;
        ParameterDict::default_str_evaluator(var, get_text());

        std::get<double>(var) *= glm::exp(0.1 * y);
        set_text(ParameterDict::default_var_formatter(var));
        editor.on_select_all();
    }
    TextCtrl::event_at(evt, at, parameter);
}