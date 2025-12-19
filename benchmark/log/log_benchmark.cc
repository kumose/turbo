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

#include <turbo/base/macros.h>
#include <turbo/base/log_severity.h>
#include <turbo/flags/flag.h>
#include <turbo/log/check.h>
#include <turbo/log/globals.h>
#include <turbo/log/flags.h>
#include <turbo/log/log.h>
#include <turbo/log/log_entry.h>
#include <turbo/log/log_sink.h>
#include <turbo/log/log_sink_registry.h>
#include <turbo/log/vlog_is_on.h>
#include <benchmark/benchmark.h>

namespace {

class NullLogSink : public turbo::LogSink {
 public:
  NullLogSink() { turbo::add_log_sink(this); }

  ~NullLogSink() override { turbo::remove_log_sink(this); }

  void Send(const turbo::LogEntry&) override {}
};

constexpr int x = -1;

void BM_SuccessfulBinaryCheck(benchmark::State& state) {
  int n = 0;
  while (state.KeepRunningBatch(8)) {
    KCHECK_GE(n, x);
    KCHECK_GE(n, x);
    KCHECK_GE(n, x);
    KCHECK_GE(n, x);
    KCHECK_GE(n, x);
    KCHECK_GE(n, x);
    KCHECK_GE(n, x);
    KCHECK_GE(n, x);
    ++n;
  }
  benchmark::DoNotOptimize(n);
}
BENCHMARK(BM_SuccessfulBinaryCheck);

static void BM_SuccessfulUnaryCheck(benchmark::State& state) {
  int n = 0;
  while (state.KeepRunningBatch(8)) {
    KCHECK(n >= x);
    KCHECK(n >= x);
    KCHECK(n >= x);
    KCHECK(n >= x);
    KCHECK(n >= x);
    KCHECK(n >= x);
    KCHECK(n >= x);
    KCHECK(n >= x);
    ++n;
  }
  benchmark::DoNotOptimize(n);
}
BENCHMARK(BM_SuccessfulUnaryCheck);

static void BM_DisabledLogOverhead(benchmark::State& state) {
  turbo::ScopedStderrThreshold disable_stderr_logging(
      turbo::LogSeverityAtLeast::kInfinity);
  turbo::log_internal::ScopedMinLogLevel scoped_min_log_level(
      turbo::LogSeverityAtLeast::kInfinity);
  for (auto _ : state) {
    KLOG(INFO);
  }
}
BENCHMARK(BM_DisabledLogOverhead);

static void BM_EnabledLogOverhead(benchmark::State& state) {
  turbo::ScopedStderrThreshold stderr_logging(
      turbo::LogSeverityAtLeast::kInfinity);
  turbo::log_internal::ScopedMinLogLevel scoped_min_log_level(
      turbo::LogSeverityAtLeast::kInfo);
  TURBO_ATTRIBUTE_UNUSED NullLogSink null_sink;
  for (auto _ : state) {
    KLOG(INFO);
  }
}
BENCHMARK(BM_EnabledLogOverhead);

static void BM_VlogIsOnOverhead(benchmark::State& state) {
  // It would make sense to do this only when state.thread_index == 0,
  // but thread_index is an int on some platforms (e.g. Android) and a
  // function returning an int on others. So we just do it on all threads.
  // TODO(b/152609127): set only if thread_index == 0.
  turbo::set_flag(&FLAGS_verbosity, 0);

  while (state.KeepRunningBatch(10)) {
    benchmark::DoNotOptimize(VKLOG_IS_ON(0));  // 1
    benchmark::DoNotOptimize(VKLOG_IS_ON(0));  // 2
    benchmark::DoNotOptimize(VKLOG_IS_ON(0));  // 3
    benchmark::DoNotOptimize(VKLOG_IS_ON(0));  // 4
    benchmark::DoNotOptimize(VKLOG_IS_ON(0));  // 5
    benchmark::DoNotOptimize(VKLOG_IS_ON(0));  // 6
    benchmark::DoNotOptimize(VKLOG_IS_ON(0));  // 7
    benchmark::DoNotOptimize(VKLOG_IS_ON(0));  // 8
    benchmark::DoNotOptimize(VKLOG_IS_ON(0));  // 9
    benchmark::DoNotOptimize(VKLOG_IS_ON(0));  // 10
  }
}
BENCHMARK(BM_VlogIsOnOverhead)->ThreadRange(1, 64);

static void BM_VlogIsNotOnOverhead(benchmark::State& state) {
  // It would make sense to do this only when state.thread_index == 0,
  // but thread_index is an int on some platforms (e.g. Android) and a
  // function returning an int on others. So we just do it on all threads.
  // TODO(b/152609127): set only if thread_index == 0.
  turbo::set_flag(&FLAGS_verbosity, 0);

  while (state.KeepRunningBatch(10)) {
    benchmark::DoNotOptimize(VKLOG_IS_ON(1));  // 1
    benchmark::DoNotOptimize(VKLOG_IS_ON(1));  // 2
    benchmark::DoNotOptimize(VKLOG_IS_ON(1));  // 3
    benchmark::DoNotOptimize(VKLOG_IS_ON(1));  // 4
    benchmark::DoNotOptimize(VKLOG_IS_ON(1));  // 5
    benchmark::DoNotOptimize(VKLOG_IS_ON(1));  // 6
    benchmark::DoNotOptimize(VKLOG_IS_ON(1));  // 7
    benchmark::DoNotOptimize(VKLOG_IS_ON(1));  // 8
    benchmark::DoNotOptimize(VKLOG_IS_ON(1));  // 9
    benchmark::DoNotOptimize(VKLOG_IS_ON(1));  // 10
  }
}
BENCHMARK(BM_VlogIsNotOnOverhead)->ThreadRange(1, 64);

static void BM_LogEveryNOverhead(benchmark::State& state) {
  turbo::ScopedStderrThreshold disable_stderr_logging(
      turbo::LogSeverityAtLeast::kInfinity);
  turbo::set_min_log_level(turbo::LogSeverityAtLeast::kInfinity);
  TURBO_ATTRIBUTE_UNUSED NullLogSink null_sink;

  while (state.KeepRunningBatch(10)) {
    KLOG_EVERY_N_SEC(INFO, 10);
    KLOG_EVERY_N_SEC(INFO, 20);
    KLOG_EVERY_N_SEC(INFO, 30);
    KLOG_EVERY_N_SEC(INFO, 40);
    KLOG_EVERY_N_SEC(INFO, 50);
    KLOG_EVERY_N_SEC(INFO, 60);
    KLOG_EVERY_N_SEC(INFO, 70);
    KLOG_EVERY_N_SEC(INFO, 80);
    KLOG_EVERY_N_SEC(INFO, 90);
    KLOG_EVERY_N_SEC(INFO, 100);
  }
}
BENCHMARK(BM_LogEveryNOverhead)->ThreadRange(1, 64);

}  // namespace

