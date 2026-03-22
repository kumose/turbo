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

#include <turbo/flags/cli/encoding.h>
#include <turbo/base/macros.h>
#include <array>
#include <clocale>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <locale>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

namespace turbo::cli {

    namespace detail {


        TURBO_DIAGNOSTIC_PUSH
        TURBO_DIAGNOSTIC_IGNORE_DEPRECATED

        std::string narrow_impl(const wchar_t *str, std::size_t str_size) {
#ifdef _WIN32
            return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().to_bytes(str, str + str_size);

#else
            return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(str, str + str_size);

#endif  // _WIN32
        }

        std::wstring widen_impl(const char *str, std::size_t str_size) {
#ifdef _WIN32
            return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().from_bytes(str, str + str_size);

#else
            return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(str, str + str_size);

#endif  // _WIN32
        }

        TURBO_DIAGNOSTIC_POP

    }  // namespace detail

    std::string narrow(const wchar_t *str, std::size_t str_size) { return detail::narrow_impl(str, str_size); }

    std::string narrow(const std::wstring &str) { return detail::narrow_impl(str.data(), str.size()); }

    // Flawfinder: ignore
    std::string narrow(const wchar_t *str) { return detail::narrow_impl(str, std::wcslen(str)); }

    std::wstring widen(const char *str, std::size_t str_size) { return detail::widen_impl(str, str_size); }

    std::wstring widen(const std::string &str) { return detail::widen_impl(str.data(), str.size()); }

    // Flawfinder: ignore
    std::wstring widen(const char *str) { return detail::widen_impl(str, std::strlen(str)); }

    std::string narrow(std::wstring_view str) { return detail::narrow_impl(str.data(), str.size()); }

    std::wstring widen(std::string_view str) { return detail::widen_impl(str.data(), str.size()); }

}  // namespace turbo::cli
