// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <turbo/log/internal/check_op.h>

#include <string.h>

#include <ostream>

#include <turbo/strings/string_view.h>

#ifdef _MSC_VER
#define strcasecmp _stricmp
#else

#include <strings.h>  // for strcasecmp, but msvc does not have this header

#endif

#include <sstream>
#include <string>

#include <turbo/base/macros.h>
#include <turbo/strings/str_cat.h>

namespace turbo::log_internal {

#define TURBO_LOGGING_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING(x) \
  template std::string* MakeCheckOpString(x, x, const char*)

    TURBO_LOGGING_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING(bool);

    TURBO_LOGGING_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING(int64_t);

    TURBO_LOGGING_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING(uint64_t);

    TURBO_LOGGING_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING(float);

    TURBO_LOGGING_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING(double);

    TURBO_LOGGING_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING(char);

    TURBO_LOGGING_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING(unsigned char);

    TURBO_LOGGING_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING(const std::string&);

    TURBO_LOGGING_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING(const std::string_view&);

    TURBO_LOGGING_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING(const char*);

    TURBO_LOGGING_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING(const signed char*);

    TURBO_LOGGING_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING(const unsigned char*);

    TURBO_LOGGING_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING(const void*);

#undef TURBO_LOGGING_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING

    CheckOpMessageBuilder::CheckOpMessageBuilder(const char *exprtext) {
        stream_ << exprtext << " (";
    }

    std::ostream &CheckOpMessageBuilder::ForVar2() {
        stream_ << " vs. ";
        return stream_;
    }

    std::string *CheckOpMessageBuilder::NewString() {
        stream_ << ")";
        return new std::string(stream_.str());
    }

    void MakeCheckOpValueString(std::ostream &os, const char v) {
        if (v >= 32 && v <= 126) {
            os << "'" << v << "'";
        } else {
            os << "char value " << int{v};
        }
    }

    void MakeCheckOpValueString(std::ostream &os, const signed char v) {
        if (v >= 32 && v <= 126) {
            os << "'" << v << "'";
        } else {
            os << "signed char value " << int{v};
        }
    }

    void MakeCheckOpValueString(std::ostream &os, const unsigned char v) {
        if (v >= 32 && v <= 126) {
            os << "'" << v << "'";
        } else {
            os << "unsigned char value " << int{v};
        }
    }

    void MakeCheckOpValueString(std::ostream &os, const void *p) {
        if (p == nullptr) {
            os << "(null)";
        } else {
            os << p;
        }
    }

// Helper functions for string comparisons.
#define DEFINE_CHECK_STROP_IMPL(name, func, expected)                      \
  std::string* Check##func##expected##Impl(const char* s1, const char* s2, \
                                           const char* exprtext) {         \
    bool equal = s1 == s2 || (s1 && s2 && !func(s1, s2));                  \
    if (equal == expected) {                                               \
      return nullptr;                                                      \
    } else {                                                               \
      return new std::string(                                              \
          turbo::str_cat(exprtext, " (", s1, " vs. ", s2, ")"));             \
    }                                                                      \
  }

    DEFINE_CHECK_STROP_IMPL(KCHECK_STREQ, strcmp, true)

    DEFINE_CHECK_STROP_IMPL(KCHECK_STRNE, strcmp, false)

    DEFINE_CHECK_STROP_IMPL(KCHECK_STRCASEEQ, strcasecmp, true)

    DEFINE_CHECK_STROP_IMPL(KCHECK_STRCASENE, strcasecmp, false)

#undef DEFINE_CHECK_STROP_IMPL

    namespace detect_specialization {

        StringifySink::StringifySink(std::ostream &os) : os_(os) {}

        void StringifySink::Append(std::string_view text) { os_ << text; }

        void StringifySink::Append(size_t length, char ch) {
            for (size_t i = 0; i < length; ++i) os_.put(ch);
        }

        void turbo_format_flush(StringifySink *sink, std::string_view text) {
            sink->Append(text);
        }

    }  // namespace detect_specialization

}  // namespace turbo::log_internal
