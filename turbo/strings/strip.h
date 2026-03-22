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
// File: strip.h
// -----------------------------------------------------------------------------
//
// This file contains various functions for stripping substrings from a string.

#pragma once

#include <cstddef>
#include <string>

#include <turbo/base/macros.h>
#include <turbo/base/nullability.h>
#include <turbo/strings/ascii.h>
#include <turbo/strings/match.h>
#include <turbo/strings/string_view.h>

namespace turbo {

    // consume_prefix()
    //
    // Strips the `expected` prefix, if found, from the start of `str`.
    // If the operation succeeded, `true` is returned.  If not, `false`
    // is returned and `str` is not modified.
    //
    // Example:
    //
    //   std::string_view input("abc");
    //   EXPECT_TRUE(turbo::consume_prefix(&input, "a"));
    //   EXPECT_EQ(input, "bc");
    inline bool consume_prefix(turbo::Nonnull<std::string_view *> str,
                              std::string_view expected) {
        if (!turbo::starts_with(*str, expected)) return false;
        str->remove_prefix(expected.size());
        return true;
    }

    // consume_suffix()
    //
    // Strips the `expected` suffix, if found, from the end of `str`.
    // If the operation succeeded, `true` is returned.  If not, `false`
    // is returned and `str` is not modified.
    //
    // Example:
    //
    //   std::string_view input("abcdef");
    //   EXPECT_TRUE(turbo::consume_suffix(&input, "def"));
    //   EXPECT_EQ(input, "abc");
    inline bool consume_suffix(turbo::Nonnull<std::string_view *> str,
                              std::string_view expected) {
        if (!turbo::ends_with(*str, expected)) return false;
        str->remove_suffix(expected.size());
        return true;
    }

    // strip_prefix()
    //
    // Returns a view into the input string `str` with the given `prefix` removed,
    // but leaving the original string intact. If the prefix does not match at the
    // start of the string, returns the original string instead.
    TURBO_MUST_USE_RESULT inline std::string_view strip_prefix(
            std::string_view str, std::string_view prefix) {
        if (turbo::starts_with(str, prefix)) str.remove_prefix(prefix.size());
        return str;
    }

    // strip_suffix()
    //
    // Returns a view into the input string `str` with the given `suffix` removed,
    // but leaving the original string intact. If the suffix does not match at the
    // end of the string, returns the original string instead.
    TURBO_MUST_USE_RESULT inline std::string_view strip_suffix(
            std::string_view str, std::string_view suffix) {
        if (turbo::ends_with(str, suffix)) str.remove_suffix(suffix.size());
        return str;
    }

}  // namespace turbo
