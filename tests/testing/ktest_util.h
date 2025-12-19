// Copyright (C) 2024 Kumo inc.
// Author: Jeff.li lijippy@163.com
// All rights reserved.
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include <turbo/utility/status.h>
#include <turbo/utility/signal.h>
#include <turbo/base/macros.h>
#include <turbo/strings/string_builder.h>

#define ASSERT_OK(expr)                                                              \
  for (::turbo::Status _st = ::turbo::internal::generic_to_status((expr)); !_st.ok();) \
  FAIL() << "'" TURBO_STRINGIFY(expr) "' failed with " << _st.to_string()

#define ASSERT_OK_NO_THROW(expr) ASSERT_NO_THROW(ASSERT_OK(expr))

#define TURBO_EXPECT_OK(expr)                                           \
  do {                                                                  \
    auto _res = (expr);                                                 \
    ::turbo::Status _st = ::turbo::internal::generic_to_status(_res);     \
    EXPECT_TRUE(_st.ok()) << "'" TURBO_STRINGIFY(expr) "' failed with " \
                          << _st.to_string();                            \
  } while (false)

#define ASSERT_NOT_OK(expr)                                                         \
  for (::turbo::Status _st = ::turbo::internal::generic_to_status((expr)); _st.ok();) \
  FAIL() << "'" TURBO_STRINGIFY(expr) "' did not failed" << _st.to_string()

#define ABORT_NOT_OK(expr)                                          \
  do {                                                              \
    auto _res = (expr);                                             \
    ::turbo::Status _st = ::turbo::internal::generic_to_status(_res); \
    if (TURBO_UNLIKELY(!_st.ok())) {                           \
      _st.abort();                                                  \
    }                                                               \
  } while (false);

#define ASSIGN_OR_HANDLE_ERROR_IMPL(handle_error, status_name, lhs, rexpr) \
  auto&& status_name = (rexpr);                                            \
  handle_error(status_name.status());                                      \
  lhs = std::move(status_name).value_or_die();

#define ASSERT_OK_AND_ASSIGN(lhs, rexpr) \
  ASSIGN_OR_HANDLE_ERROR_IMPL(           \
      ASSERT_OK, TURBO_CONCAT(_error_or_value, __COUNTER__), lhs, rexpr);

#define ASSIGN_OR_ABORT(lhs, rexpr)                                                     \
  ASSIGN_OR_HANDLE_ERROR_IMPL(ABORT_NOT_OK,                                             \
                              TURBO_CONCAT(_error_or_value, __COUNTER__), \
                              lhs, rexpr);

#define EXPECT_OK_AND_ASSIGN(lhs, rexpr)                                                \
  ASSIGN_OR_HANDLE_ERROR_IMPL(TURBO_EXPECT_OK,                                          \
                              TURBO_CONCAT(_error_or_value, __COUNTER__), \
                              lhs, rexpr);

#define ASSERT_OK_AND_EQ(expected, expr)        \
  do {                                          \
    ASSERT_OK_AND_ASSIGN(auto _actual, (expr)); \
    ASSERT_EQ(expected, _actual);               \
  } while (0)


namespace turbo {

}