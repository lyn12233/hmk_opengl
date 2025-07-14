#pragma once

#include "config.hxx"
#include "sizer.hxx"
#include "textedit.hxx"
#include "utils.hxx"

#include <cassert>
#include <functional>
#include <map>
#include <memory>
#include <spdlog/spdlog.h>
#include <stdint.h>
#include <type_traits>
#include <variant>

namespace mf {
    using glm::vec3;
    using std::map;
    using std::shared_ptr;
    using std::string;
    using std::variant;

    typedef variant<int64_t, double, string, vec3> ParameterDictVar_t;
    typedef map<string, ParameterDictVar_t>        ParameterDict_t;

    enum PDTYPE : int32_t {
        PD_ANY    = -1,
        PD_INT64  = 0,
        PD_DOUBLE = 1,
        PD_STR    = 2,
        PD_VEC3   = 3,
    };

    class ParameterDict : public BoxSizer {

        public:
        ParameterDict(int entry_per_col = DEFAULT_PARM_PER_COL);
        ParameterDict(ParameterDict_t dict, int entry_per_col = DEFAULT_PARM_PER_COL);
        ~ParameterDict() override;

        void add(string name, ParameterDictVar_t value);
        void add(ParameterDict_t many);

        inline auto  data() { return data_; }
        inline auto &operator[](string name) {
            if (!data_.count(name)) {
                spdlog::error("ParameterDict: parm '{}' not exist", name);
                exit(-1);
            }
            return data_[name];
        }

        inline bool query_content_changed() {
            bool ret         = content_changed_;
            content_changed_ = false;
            return ret;
        }

        // protected:
        ParameterDict_t data_;

        int                  entry_per_col_;
        shared_ptr<BoxSizer> acquire_line_();

        bool content_changed_;

        static string default_var_formatter(const ParameterDictVar_t &var);
        static bool   default_var_validator(const ParameterDictVar_t &var);
        static string default_str_evaluator(ParameterDictVar_t &var, string text);
    };

    class ParameterTC : public TextCtrl {
        public:
        ParameterTC(
            std::string text = "", PDTYPE type = PD_ANY, GLuint w = DEFAULT_TC_WIDTH, GLuint h = 20,
            GLuint    fontsize = DEFAULT_TC_FONTSIZE,
            mf::FLAGS style    = static_cast<FLAGS>(mf::ALIGN_CENTER | mf::EXPAND)
        );
        ~ParameterTC() override;
        void   event_at(EVENT evt, Pos at, EVENT_PARM parameter) override;
        PDTYPE pd_type_;
    };
} // namespace mf