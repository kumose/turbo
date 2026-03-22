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
//
// -----------------------------------------------------------------------------
// File: match.h
// -----------------------------------------------------------------------------
//
// This file contains simple utilities for performing string matching checks.
// All of these function parameters are specified as `std::string_view`,
// meaning that these functions can accept `std::string`, `std::string_view` or
// NUL-terminated C-style strings.
//
// Examples:
//   std::string s = "foo";
//   std::string_view sv = "f";
//   assert(turbo::str_contains(s, sv));
//
// Note: The order of parameters in these functions is designed to mimic the
// order an equivalent member function would exhibit;
// e.g. `s.Contains(x)` ==> `turbo::str_contains(s, x).

#pragma once

#include <cstring>

#include <turbo/strings/string_view.h>

namespace turbo {

    // str_contains()
    //
    // Returns whether a given string `haystack` contains the substring `needle`.
    inline bool str_contains(std::string_view haystack,
                            std::string_view needle) noexcept {
        return haystack.find(needle, 0) != haystack.npos;
    }

    inline bool str_contains(std::string_view haystack, char needle) noexcept {
        return haystack.find(needle) != haystack.npos;
    }

    // starts_with()
    //
    // Returns whether a given string `text` begins with `prefix`.
    inline bool starts_with(std::string_view text,
                           std::string_view prefix) noexcept {
        return prefix.empty() ||
               (text.size() >= prefix.size() &&
                memcmp(text.data(), prefix.data(), prefix.size()) == 0);
    }

    // ends_with()
    //
    // Returns whether a given string `text` ends with `suffix`.
    inline bool ends_with(std::string_view text,
                         std::string_view suffix) noexcept {
        return suffix.empty() ||
               (text.size() >= suffix.size() &&
                memcmp(text.data() + (text.size() - suffix.size()), suffix.data(),
                       suffix.size()) == 0);
    }

    // str_contains_ignore_case()
    //
    // Returns whether a given ASCII string `haystack` contains the ASCII substring
    // `needle`, ignoring case in the comparison.
    bool str_contains_ignore_case(std::string_view haystack,
                               std::string_view needle) noexcept;

    bool str_contains_ignore_case(std::string_view haystack,
                               char needle) noexcept;

    // equals_ignore_case()
    //
    // Returns whether given ASCII strings `piece1` and `piece2` are equal, ignoring
    // case in the comparison.
    bool equals_ignore_case(std::string_view piece1,
                          std::string_view piece2) noexcept;

    // starts_with_ignore_case()
    //
    // Returns whether a given ASCII string `text` starts with `prefix`,
    // ignoring case in the comparison.
    bool starts_with_ignore_case(std::string_view text,
                              std::string_view prefix) noexcept;

    // ends_with_ignore_case()
    //
    // Returns whether a given ASCII string `text` ends with `suffix`, ignoring
    // case in the comparison.
    bool ends_with_ignore_case(std::string_view text,
                            std::string_view suffix) noexcept;

    // Yields the longest prefix in common between both input strings.
    // Pointer-wise, the returned result is a subset of input "a".
    std::string_view find_longest_common_prefix(std::string_view a,
                                               std::string_view b);

    // Yields the longest suffix in common between both input strings.
    // Pointer-wise, the returned result is a subset of input "a".
    std::string_view find_longest_common_suffix(std::string_view a,
                                               std::string_view b);

}  // namespace turbo
