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

ParameterDict::ParameterDict(int entry_per_col) : BoxSizer(), entry_per_col_(entry_per_col) {}

ParameterDict::ParameterDict(ParameterDict_t dict, int entry_per_col) :
    BoxSizer(), entry_per_col_(entry_per_col) {
    add(dict);
}

ParameterDict::~ParameterDict() {}

void ParameterDict::add(string name, ParameterDictVar_t value) { //
    auto vsizer  = acquire_line_();
    auto hsizer  = std::make_shared<BoxSizer>(0, 30, 0, mf::SIZER_HORIZONTAL);
    auto text    = std::make_shared<StaticText>(name);
    auto var_str = std::make_shared<TextCtrl>(
        default_var_validator(value) ? default_var_formatter(value) : ""
    );
    hsizer->add(text, 1.);
    hsizer->add(var_str, 2.);
    vsizer->add(hsizer, 0.);

    auto callback = [this, text, var_str](EVENT, Pos, EVENT_PARM) { //
        this->content_changed_ = true;
        auto &last_var         = this->data_[text->get_text()];
        auto  str              = var_str->get_text();
        auto  new_str          = default_str_evaluator(last_var, str);
        var_str->set_text(new_str);
    };
    var_str->bind_event(EVT_FOCUS_OUT, callback);

    data_[name] = value;
}

void ParameterDict::add(ParameterDict_t many) {
    for (const auto &[name, value] : many) {
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
        return default_var_formatter(std::atoll(text.c_str()));
    } else if (std::holds_alternative<double>(var)) {
        return default_var_formatter(std::atof(text.c_str()));
    } else if (std::holds_alternative<string>(var)) {
        return text;
    } else if (std::holds_alternative<vec3>(var)) {
        float x = 0, y = 0, z = 0;
        std::sscanf(text.c_str(), "%f,%f,%f", &x, &y, &z);
        return default_var_formatter(vec3(x, y, z));
    }
    return "??";
}