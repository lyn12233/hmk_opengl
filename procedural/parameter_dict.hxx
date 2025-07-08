#pragma once

#include "sizer.hxx"

#include <functional>
#include <map>
#include <memory>
#include <stdint.h>
#include <type_traits>
#include <variant>

namespace mf {
    using glm::vec3;
    using std::map;
    using std::shared_ptr;
    using std::string;
    using std::variant;

    class ParameterDict : public BoxSizer {

        typedef variant<int64_t, double, string, vec3> ParameterDictVar_t;
        typedef map<string, ParameterDictVar_t>        ParameterDict_t;

        public:
        ParameterDict(int entry_per_col = 10);
        ParameterDict(ParameterDict_t dict, int entry_per_col = 10);
        ~ParameterDict() override;

        void add(string name, ParameterDictVar_t value);
        void add(ParameterDict_t many);

        inline auto data() { return data_; }
        inline auto operator[](string name) { return data_[name]; }
        inline bool query_content_changed() {
            bool ret         = content_changed_;
            content_changed_ = false;
            return ret;
        }

        protected:
        ParameterDict_t data_;

        int                  entry_per_col_;
        shared_ptr<BoxSizer> acquire_line_();

        bool content_changed_;

        static string default_var_formatter(const ParameterDictVar_t &var);
        static bool   default_var_validator(const ParameterDictVar_t &var);
        static string default_str_evaluator(ParameterDictVar_t &var, string text);
    };
} // namespace mf