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

#include <type_traits>
#include <utility>

#include <turbo/base/macros.h>
#include <turbo/strings/string_view.h>

namespace turbo {

    namespace strings_internal {

        // This is an empty class not intended to be used. It exists so that
        // `HasTurboStringify` can reference a universal class rather than needing to be
        // copied for each new sink.
        class UnimplementedSink {
        public:
            void Append(size_t count, char ch);

            void Append(std::string_view v);

            // Support `turbo::format(&sink, format, args...)`.
            friend void turbo_format_flush(UnimplementedSink *sink, std::string_view v);
        };

    }  // namespace strings_internal

    // `HasTurboStringify<T>` detects if type `T` supports the `turbo_stringify()`
    //
    // Note that there are types that can be `str_cat`-ed that do not use the
    // `turbo_stringify` customization point (for example, `int`).

    template<typename T, typename = void>
    struct HasTurboStringify : std::false_type {
    };

    template<typename T>
    struct HasTurboStringify<
            T, std::enable_if_t<std::is_void<decltype(turbo_stringify(
                    std::declval<strings_internal::UnimplementedSink &>(),
                    std::declval<const T &>()))>::value>> : std::true_type {
    };

    template<typename T, typename = void>
    struct HasToStringMember : std::false_type {
    };

    template<typename T>
    struct HasToStringMember<
            T, std::enable_if_t<
                    std::is_same<decltype(std::declval<const T>().to_string()), std::string>::value ||
                    std::is_same<decltype(std::declval<const T>().to_string()), const std::string &>::value ||
                    std::is_same<decltype(std::declval<const T>().to_string()), std::string_view>::value
            >>
            : std::true_type {
    };

    template<typename T, typename = void>
    struct HasStdToString : std::false_type {
    };

    template<typename T>
    struct HasStdToString<
            T, std::enable_if_t<std::is_same<decltype(std::to_string(std::declval<const T &>())), std::string>::value>>
            : std::true_type {
    };

    template<typename T, typename = void>
    struct HasDescribe : std::false_type {
    };

    template<typename T>
    struct HasDescribe<
            T, std::enable_if_t<std::is_same<decltype(std::declval<const T>().describe(
                    std::declval<std::ostream &>())), void>::value>>
            : std::true_type {
    };

}  // namespace turbo
