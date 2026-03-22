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

#include <turbo/log/vlog_is_on.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <turbo/base/log_severity.h>
#include <turbo/flags/flag.h>
#include <turbo/log/flags.h>
#include <turbo/log/globals.h>
#include <turbo/log/log.h>
#include <tests/log/scoped_mock_log.h>
#include <optional>

namespace {

using ::testing::_;

std::optional<int> MaxLogVerbosity() {
#ifdef TURBO_MAX_VLOG_VERBOSITY
  return TURBO_MAX_VLOG_VERBOSITY;
#else
  return std::nullopt;
#endif
}

std::optional<int> min_log_level() {
#ifdef TURBO_MIN_LOG_LEVEL
  return static_cast<int>(TURBO_MIN_LOG_LEVEL);
#else
  return std::nullopt;
#endif
}

TEST(VLogIsOn, GlobalWorksWithoutMaxVerbosityAndMinLogLevel) {
  if (MaxLogVerbosity().has_value() || min_log_level().has_value()) {
    GTEST_SKIP();
  }

  turbo::set_global_vlog_level(3);
  turbo::ScopedMockLog log(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(log, Log(turbo::LogSeverity::kInfo, _, "important"));

  log.StartCapturingLogs();
  VKLOG(3) << "important";
  VKLOG(4) << "spam";
}

TEST(VLogIsOn, FileWorksWithoutMaxVerbosityAndMinLogLevel) {
  if (MaxLogVerbosity().has_value() || min_log_level().has_value()) {
    GTEST_SKIP();
  }

  turbo::set_vlog_level("vlog_is_on_test", 3);
  turbo::ScopedMockLog log(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(log, Log(turbo::LogSeverity::kInfo, _, "important"));

  log.StartCapturingLogs();
  VKLOG(3) << "important";
  VKLOG(4) << "spam";
}

TEST(VLogIsOn, PatternWorksWithoutMaxVerbosityAndMinLogLevel) {
  if (MaxLogVerbosity().has_value() || min_log_level().has_value()) {
    GTEST_SKIP();
  }

  turbo::set_vlog_level("vlog_is_on*", 3);
  turbo::ScopedMockLog log(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(log, Log(turbo::LogSeverity::kInfo, _, "important"));

  log.StartCapturingLogs();
  VKLOG(3) << "important";
  VKLOG(4) << "spam";
}

TEST(VLogIsOn, GlobalDoesNotFilterBelowMaxVerbosity) {
  if (!MaxLogVerbosity().has_value() || *MaxLogVerbosity() < 2) {
    GTEST_SKIP();
  }

  // Set an arbitrary high value to avoid filtering VLOGs in tests by default.
  turbo::set_global_vlog_level(1000);
  turbo::ScopedMockLog log(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(log, Log(turbo::LogSeverity::kInfo, _, "asdf"));

  log.StartCapturingLogs();
  VKLOG(2) << "asdf";
}

TEST(VLogIsOn, FileDoesNotFilterBelowMaxVerbosity) {
  if (!MaxLogVerbosity().has_value() || *MaxLogVerbosity() < 2) {
    GTEST_SKIP();
  }

  // Set an arbitrary high value to avoid filtering VLOGs in tests by default.
  turbo::set_vlog_level("vlog_is_on_test", 1000);
  turbo::ScopedMockLog log(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(log, Log(turbo::LogSeverity::kInfo, _, "asdf"));

  log.StartCapturingLogs();
  VKLOG(2) << "asdf";
}

TEST(VLogIsOn, PatternDoesNotFilterBelowMaxVerbosity) {
  if (!MaxLogVerbosity().has_value() || *MaxLogVerbosity() < 2) {
    GTEST_SKIP();
  }

  // Set an arbitrary high value to avoid filtering VLOGs in tests by default.
  turbo::set_vlog_level("vlog_is_on*", 1000);
  turbo::ScopedMockLog log(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(log, Log(turbo::LogSeverity::kInfo, _, "asdf"));

  log.StartCapturingLogs();
  VKLOG(2) << "asdf";
}

TEST(VLogIsOn, GlobalFiltersAboveMaxVerbosity) {
  if (!MaxLogVerbosity().has_value() || *MaxLogVerbosity() >= 4) {
    GTEST_SKIP();
  }

  // Set an arbitrary high value to avoid filtering VLOGs in tests by default.
  turbo::set_global_vlog_level(1000);
  turbo::ScopedMockLog log(turbo::MockLogDefault::kDisallowUnexpected);

  log.StartCapturingLogs();
  VKLOG(4) << "dfgh";
}

TEST(VLogIsOn, FileFiltersAboveMaxVerbosity) {
  if (!MaxLogVerbosity().has_value() || *MaxLogVerbosity() >= 4) {
    GTEST_SKIP();
  }

  // Set an arbitrary high value to avoid filtering VLOGs in tests by default.
  turbo::set_vlog_level("vlog_is_on_test", 1000);
  turbo::ScopedMockLog log(turbo::MockLogDefault::kDisallowUnexpected);

  log.StartCapturingLogs();
  VKLOG(4) << "dfgh";
}

TEST(VLogIsOn, PatternFiltersAboveMaxVerbosity) {
  if (!MaxLogVerbosity().has_value() || *MaxLogVerbosity() >= 4) {
    GTEST_SKIP();
  }

  // Set an arbitrary high value to avoid filtering VLOGs in tests by default.
  turbo::set_vlog_level("vlog_is_on*", 1000);
  turbo::ScopedMockLog log(turbo::MockLogDefault::kDisallowUnexpected);

  log.StartCapturingLogs();
  VKLOG(4) << "dfgh";
}

}  // namespace
