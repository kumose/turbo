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