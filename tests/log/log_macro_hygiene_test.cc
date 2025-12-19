//
// Copyright (C) 2024 EA group inc.
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <turbo/base/macros.h>
#include <turbo/base/log_severity.h>
#include <turbo/log/log.h>
#include <tests/log/scoped_mock_log.h>

namespace {
using ::testing::_;
using ::testing::Eq;

namespace not_turbo {

class Dummy {
 public:
  Dummy() {}

 private:
  Dummy(const Dummy&) = delete;
  Dummy& operator=(const Dummy&) = delete;
};

// This line tests that local definitions of INFO, WARNING, ERROR, and
// etc don't shadow the global ones used by the logging macros.  If
// they do, the KLOG() calls in the tests won't compile, catching the
// bug.
const Dummy INFO, WARNING, ERROR, FATAL, NUM_SEVERITIES;

// These makes sure that the uses of same-named types in the
// implementation of the logging macros are fully qualified.
class string {};
class vector {};
class LogMessage {};
class LogMessageFatal {};
class LogMessageQuietlyFatal {};
class LogMessageVoidify {};
class LogSink {};
class NullStream {};
class NullStreamFatal {};

}  // namespace not_turbo

using namespace not_turbo;  // NOLINT

// Tests for KLOG(LEVEL(()).

TEST(LogHygieneTest, WorksForQualifiedSeverity) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);

  ::testing::InSequence seq;
  EXPECT_CALL(test_sink, Log(turbo::LogSeverity::kInfo, _, "To INFO"));
  EXPECT_CALL(test_sink, Log(turbo::LogSeverity::kWarning, _, "To WARNING"));
  EXPECT_CALL(test_sink, Log(turbo::LogSeverity::kError, _, "To ERROR"));

  test_sink.StartCapturingLogs();
  // Note that KLOG(LEVEL()) expects the severity as a run-time
  // expression (as opposed to a compile-time constant).  Hence we
  // test that :: is allowed before INFO, etc.
  KLOG(LEVEL(turbo::LogSeverity::kInfo)) << "To INFO";
  KLOG(LEVEL(turbo::LogSeverity::kWarning)) << "To WARNING";
  KLOG(LEVEL(turbo::LogSeverity::kError)) << "To ERROR";
}

TEST(LogHygieneTest, WorksWithAlternativeINFOSymbol) {
  const double INFO TURBO_ATTRIBUTE_UNUSED = 7.77;
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(test_sink, Log(turbo::LogSeverity::kInfo, _, "Hello world"));

  test_sink.StartCapturingLogs();
  KLOG(INFO) << "Hello world";
}

TEST(LogHygieneTest, WorksWithAlternativeWARNINGSymbol) {
  const double WARNING TURBO_ATTRIBUTE_UNUSED = 7.77;
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(test_sink, Log(turbo::LogSeverity::kWarning, _, "Hello world"));

  test_sink.StartCapturingLogs();
  KLOG(WARNING) << "Hello world";
}

TEST(LogHygieneTest, WorksWithAlternativeERRORSymbol) {
  const double ERROR TURBO_ATTRIBUTE_UNUSED = 7.77;
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(test_sink, Log(turbo::LogSeverity::kError, _, "Hello world"));

  test_sink.StartCapturingLogs();
  KLOG(ERROR) << "Hello world";
}

TEST(LogHygieneTest, WorksWithAlternativeLEVELSymbol) {
  const double LEVEL TURBO_ATTRIBUTE_UNUSED = 7.77;
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(test_sink, Log(turbo::LogSeverity::kError, _, "Hello world"));

  test_sink.StartCapturingLogs();
  KLOG(LEVEL(turbo::LogSeverity::kError)) << "Hello world";
}

#define INFO Bogus
#ifdef NDEBUG
constexpr bool IsOptimized = false;
#else
constexpr bool IsOptimized = true;
#endif

TEST(LogHygieneTest, WorksWithINFODefined) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(test_sink, Log(turbo::LogSeverity::kInfo, _, "Hello world"))
      .Times(2 + (IsOptimized ? 2 : 0));

  test_sink.StartCapturingLogs();
  KLOG(INFO) << "Hello world";
  KLOG_IF(INFO, true) << "Hello world";

  DKLOG(INFO) << "Hello world";
  DKLOG_IF(INFO, true) << "Hello world";
}

#undef INFO

#define _INFO Bogus
TEST(LogHygieneTest, WorksWithUnderscoreINFODefined) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(test_sink, Log(turbo::LogSeverity::kInfo, _, "Hello world"))
      .Times(2 + (IsOptimized ? 2 : 0));

  test_sink.StartCapturingLogs();
  KLOG(INFO) << "Hello world";
  KLOG_IF(INFO, true) << "Hello world";

  DKLOG(INFO) << "Hello world";
  DKLOG_IF(INFO, true) << "Hello world";
}
#undef _INFO

TEST(LogHygieneTest, ExpressionEvaluationInLEVELSeverity) {
  auto i = static_cast<int>(turbo::LogSeverity::kInfo);
  KLOG(LEVEL(++i)) << "hello world";  // NOLINT
  EXPECT_THAT(i, Eq(static_cast<int>(turbo::LogSeverity::kInfo) + 1));
}

TEST(LogHygieneTest, ExpressionEvaluationInStreamedMessage) {
  int i = 0;
  KLOG(INFO) << ++i;
  EXPECT_THAT(i, 1);
  KLOG_IF(INFO, false) << ++i;
  EXPECT_THAT(i, 1);
}

// Tests that macros are usable in unbraced switch statements.
// -----------------------------------------------------------

class UnbracedSwitchCompileTest {
  static void Log() {
    switch (0) {
      case 0:
        KLOG(INFO);
        break;
      default:
        break;
    }
  }
};

}  // namespace
