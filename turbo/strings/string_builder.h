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



#pragma once

#include <memory>
#include <ostream>
#include <iostream>
#include <string>
#include <utility>
#include <turbo/strings/str_cat.h>
#include <turbo/strings/internal/ostringstream.h>
#include <turbo/strings/has_stringify.h>
#include <turbo/strings/has_ostream_operator.h>
#include <turbo/base/macros.h>
#include <turbo/strings/str_format.h>
#include <turbo/strings/substitute.h>

namespace turbo {

    template <typename T>
    struct IsTurboFormatAble{
        static constexpr bool value = HasOstreamOperator<T>::value ||
                HasDescribe<T>::value ||
                HasTurboStringify<T>::value ||
                HasToStringMember<T>::value;
    };


    class StringBuilder {
    public:
        StringBuilder()
                : _storage(),
                  _stream(&_storage) {

        }

        ~StringBuilder() = default;


        const std::string &str() const &{
            return _storage;
        }

        std::string str() &&{
            auto result = std::move(_storage);
            _stream.str(&_storage);
            return result;
        }

        template<typename T>
        StringBuilder &operator<<(T &&v);

        template<typename... Args>
        static std::string create(Args &&... args);
    private:
        template<typename T>
        inline static void format_impl(turbo::strings_internal::OStringStream &stream, T &&head);

        template<typename Head>
        inline static void create_recursive(turbo::strings_internal::OStringStream &stream, Head &&head);

        inline static void create_recursive(turbo::strings_internal::OStringStream &stream);


        template<typename Head, typename... Tail>
        void
        static create_recursive(turbo::strings_internal::OStringStream &stream, Head &&head, Tail &&... tail);

    private:
        std::string _storage;
        turbo::strings_internal::OStringStream _stream;
    };

    template<typename T>
    StringBuilder &StringBuilder::operator<<(T &&v) {
        format_impl(_stream, std::forward<T>(v));
        return *this;
    }

    template<typename... Args>
    inline std::string StringBuilder::create(Args &&... args) {
        std::string result;
        turbo::strings_internal::OStringStream os(&result);
        create_recursive(os, std::forward<Args>(args)...);
        return result;
    }

    template<typename Head, typename... Tail>
    void
    inline StringBuilder::create_recursive(turbo::strings_internal::OStringStream &stream, Head &&head,
                                    Tail &&... tail) {
        create_recursive(stream, std::forward<Head>(head));
        create_recursive(stream, std::forward<Tail>(tail)...);
    }

    template<typename Head>
    inline void StringBuilder::create_recursive(turbo::strings_internal::OStringStream &stream, Head &&head) {
        format_impl(stream, std::forward<Head>(head));
    }

    inline void StringBuilder::create_recursive(turbo::strings_internal::OStringStream &stream) {
        TURBO_UNUSED(stream);
    }

    template<typename T>
    inline void StringBuilder::format_impl(turbo::strings_internal::OStringStream &stream, T &&head) {
        if constexpr (HasOstreamOperator<T>::value) {
            stream << head;
        } else if constexpr (HasDescribe<T>::value) {
            head.describe(stream);
        } else if constexpr (HasTurboStringify<T>::value) {
            stream << std::forward<std::string>(turbo::str_cat(head));
        } else if constexpr (HasToStringMember<T>::value) {
            stream << std::forward<std::string>(head.to_string());
        } else {
            static_assert(IsTurboFormatAble<T>::value,
                          "you must implement std::operator<< || to_string "
                          "|| describe(std::ostream &) ||"
                          "turbo_stringify"
            );
        }
    }

    /// CRTP helper for declaring string representation. Defines operator<<
    template<typename T>
    class ToStringOstreamable {
    public:
        ~ToStringOstreamable() {
            static_assert(HasToStringMember<T>::value || HasDescribe<T>::value,
                    "ToStringOstreamable depends on the method std::string T::to_string() const or"
                    "const std::string &T::to_string() const or"
                    "void T::describe(std::ostream&) const");
        }

    private:
        const T &cast() const { return static_cast<const T &>(*this); }

        friend inline std::ostream &operator<<(std::ostream &os, const T &t) {
            if constexpr (HasDescribe<T>::value) {
                t.describe(os);
                return os;
            } else {
                os<<t.to_string();
                return os;
            }
        }
    };

}  // namespace turbo

