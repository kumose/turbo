// Copyright (C) Kumo inc. and its affiliates.
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

#include <turbo/log/klog.h>
#include <turbo/log/sinks/json_file_sink.h>

// KLOG()
//
// `JSON_LOG` takes a single argument which is a severity level.  Data streamed in
// comprise the logged message.
// Example:
//    AppInfo info{appid, lig, "krpc"};
//   JSON_LOG(INFO, info) << "Found " << num_cookies << " cookies";
#define JSON_LOG(severity, info) KLOG(severity).to_sink_only(::turbo::json_log_sink())<<info
/// just for testing
#define JSON_LOG_TS(severity, info, ts) KLOG(severity).to_sink_only(::turbo::json_log_sink()).with_timestamp(ts)<<info
// JSON_PLOG()
//
// `JSON_PLOG` behaves like `KLOG` except that a description of the current state of
// `errno` is appended to the streamed message.
#define JSON_PLOG(severity, info) PKLOG(severity).to_sink_only(::turbo::json_log_sink())<<info

// JSON_DLOG()
//
// `JSON_DLOG` behaves like `KLOG` in debug mode (i.e. `#ifndef NDEBUG`).  Otherwise
// it compiles away and does nothing.  Note that `JSON_DLOG(FATAL)` does not
// terminate the program if `NDEBUG` is defined.
#define JSON_DLOG(severity, info) DKLOG(severity).to_sink_only(::turbo::json_log_sink())<<info

// `JSON_VLOG` uses numeric levels to provide verbose logging that can configured at
// runtime, including at a per-module level.  `JSON_VLOG` statements are logged at
// `INFO` severity if they are logged at all; the numeric levels are on a
// different scale than the proper severity levels.  Positive levels are
// disabled by default.  Negative levels should not be used.
// Example:
//
//   JSON_VLOG(1) << "I print when you run the program with --verbosity=1 or higher";
//   JSON_VLOG(2) << "I print when you run the program with --verbosity=2 or higher";
//
// See vlog_is_on.h for further documentation, including the usage of the
// --vlog_module flag to log at different levels in different source files.
//
// `JSON_VLOG` does not produce any output when verbose logging is not enabled.
// However, simply testing whether verbose logging is enabled can be expensive.
// If you don't intend to enable verbose logging in non-debug builds, consider
// using `JSON_DVLOG` instead.
#define JSON_VLOG(severity, info) VKLOG(severity).to_sink_only(::turbo::json_log_sink())<<info

// `JSON_DVLOG` behaves like `JSON_VLOG` in debug mode (i.e. `#ifndef NDEBUG`).
// Otherwise, it compiles away and does nothing.
#define JSON_DVLOG(severity, info) DVKLOG(severity).to_sink_only(::turbo::json_log_sink())<<info

// `JSON_LOG_IF` and friends add a second argument which specifies a condition.  If
// the condition is false, nothing is logged.
// Example:
//
//   JSON_LOG_IF(INFO, num_cookies > 10) << "Got lots of cookies";
//
// There is no `VLOG_IF` because the order of evaluation of the arguments is
// ambiguous and the alternate spelling with an `if`-statement is trivial.
#define JSON_LOG_IF(severity, info, condition) KLOG_IF(condition, severity).to_sink_only(::turbo::json_log_sink())<<info

#define JSON_PLOG_IF(severity, condition)  PKLOG_IF(severity, condition).to_sink_only(::turbo::json_log_sink())<<info
#define JSON_DLOG_IF(severity, condition)  DKLOG_IF(severity, condition).to_sink_only(::turbo::json_log_sink())<<info

// JSON_LOG_EVERY_N
//
// An instance of `JSON_LOG_EVERY_N` increments a hidden zero-initialized counter
// every time execution passes through it and logs the specified message when
// the counter's value is a multiple of `n`, doing nothing otherwise.  Each
// instance has its own counter.  The counter's value can be logged by streaming
// the symbol `COUNTER`.  `JSON_LOG_EVERY_N` is thread-safe.
// Example:
//
//   JSON_LOG_EVERY_N(WARNING, 1000) << "Got a packet with a bad CRC (" << COUNTER
//                              << " total)";
#define JSON_LOG_EVERY_N(severity, info, n) KLOG_EVERY_N(severity, n).to_sink_only(::turbo::json_log_sink())<<info

// JSON_LOG_FIRST_N
//
// `JSON_LOG_FIRST_N` behaves like `JSON_LOG_EVERY_N` except that the specified message is
// logged when the counter's value is less than `n`.  `JSON_LOG_FIRST_N` is
// thread-safe.
#define JSON_LOG_FIRST_N(severity, info, n) KLOG_FIRST_N(severity, n).to_sink_only(::turbo::json_log_sink())<<info

#define JSON_LOG_ONCE(severity, info) KLOG_ONCE(severity).to_sink_only(::turbo::json_log_sink())<<info

// JSON_LOG_EVERY_POW_2
//
// `JSON_LOG_EVERY_POW_2` behaves like `KLOG_EVERY_N` except that the specified
// message is logged when the counter's value is a power of 2.
// `JSON_LOG_EVERY_POW_2` is thread-safe.
#define JSON_LOG_EVERY_POW_2(severity, info) KLOG_EVERY_POW_2(severity).to_sink_only(::turbo::json_log_sink())<<info

// JSON_LOG_EVERY_N_SEC
//
// An instance of `JSON_LOG_EVERY_N_SEC` uses a hidden state variable to log the
// specified message at most once every `n_seconds`.  A hidden counter of
// executions (whether a message is logged or not) is also maintained and can be
// logged by streaming the symbol `COUNTER`.  `JSON_LOG_EVERY_N_SEC` is thread-safe.
// Example:
//
//   JSON_LOG_EVERY_N_SEC(INFO, 2.5) << "Got " << COUNTER << " cookies so far";
#define JSON_LOG_EVERY_N_SEC(severity, info, n_seconds) KLOG_EVERY_N_SEC(severity, n_seconds).to_sink_only(::turbo::json_log_sink())<<info

#define JSON_LOG_EVERY_SEC(severity, info)   KLOG_EVERY_SEC(severity).to_sink_only(::turbo::json_log_sink())<<info

#define JSON_LOG_EVERY_MIN(severity, info) KLOG_EVERY_MIN(severity).to_sink_only(::turbo::json_log_sink())<<info
#define JSON_PLOG_EVERY_N(severity, info, n) PKLOG_EVERY_N(severity, n).to_sink_only(::turbo::json_log_sink())<<info
#define JSON_PLOG_FIRST_N(severity, n) PKLOG_FIRST_N(severity, n)
#define JSON_PLOG_ONCE(severity, info)  PKLOG_ONCE(severity).to_sink_only(::turbo::json_log_sink())<<info
#define JSON_PLOG_EVERY_POW_2(severity, info) PKLOG_EVERY_POW_2(severity).to_sink_only(::turbo::json_log_sink())<<info

#define JSON_PLOG_EVERY_N_SEC(severity, n_seconds) PKLOG_EVERY_N_SEC(severity, n_seconds).to_sink_only(::turbo::json_log_sink())<<info
#define JSON_PLOG_EVERY_SEC(severity, info) PKLOG_EVERY_SEC(severity).to_sink_only(::turbo::json_log_sink())<<info
#define JSON_PLOG_EVERY_MIN(severity, info) PKLOG_EVERY_MIN(severity).to_sink_only(::turbo::json_log_sink())<<info

#define JSON_DLOG_EVERY_N(severity, info, n) DKLOG_EVERY_N(severity, n).to_sink_only(::turbo::json_log_sink())<<info
#define JSON_DLOG_FIRST_N(severity, info, n)  DKLOG_FIRST_N(severity, n).to_sink_only(::turbo::json_log_sink())<<info

#define JSON_DLOG_ONCE(severity, info) DKLOG_ONCE(severity).to_sink_only(::turbo::json_log_sink())<<info
#define JSON_DLOG_EVERY_POW_2(severity, info) DKLOG_EVERY_POW_2(severity).to_sink_only(::turbo::json_log_sink())<<info
#define JSON_DLOG_EVERY_N_SEC(severity, info, n_seconds) DKLOG_EVERY_N_SEC(severity, n_seconds).to_sink_only(::turbo::json_log_sink())<<info
#define JSON_DLOG_EVERY_SEC(severity, info) DKLOG_EVERY_SEC(severity).to_sink_only(::turbo::json_log_sink())<<info
#define JSON_DLOG_EVERY_MIN(severity, info) DKLOG_EVERY_MIN(severity).to_sink_only(::turbo::json_log_sink())<<info
#define JSON_VLOG_EVERY_N(severity, info, n) VKLOG_EVERY_N(severity, n).to_sink_only(::turbo::json_log_sink())<<info
#define JSON_VLOG_FIRST_N(severity, info, n) VKLOG_FIRST_N(severity, n).to_sink_only(::turbo::json_log_sink())<<info
#define JSON_VLOG_ONCE(severity, info) VKLOG_ONCE(severity).to_sink_only(::turbo::json_log_sink())<<info
#define JSON_VLOG_EVERY_POW_2(severity, info) VKLOG_EVERY_POW_2(severity).to_sink_only(::turbo::json_log_sink())<<info
#define JSON_VLOG_EVERY_N_SEC(severity, info, n_seconds) VKLOG_EVERY_N_SEC(severity, n_seconds).to_sink_only(::turbo::json_log_sink())<<info
#define JSON_VLOG_EVERY_SEC(severity, info) VKLOG_EVERY_SEC(severity).to_sink_only(::turbo::json_log_sink())<<info
#define JSON_VLOG_EVERY_MIN(severity, info) VKLOG_EVERY_MIN(severity).to_sink_only(::turbo::json_log_sink())<<info

// `JSON_LOG_IF_EVERY_N` and friends behave as the corresponding `KLOG_EVERY_N`
// but neither increment a counter nor log a message if condition is false (as
// `JSON_LOG_IF`).
// Example:
//
//   JSON_LOG_IF_EVERY_N(INFO, (size > 1024), 10) << "Got the " << COUNTER
//                                           << "th big cookie";
#define JSON_LOG_IF_EVERY_N(severity, info, condition, n) KLOG_IF_EVERY_N(severity, condition, n).to_sink_only(::turbo::json_log_sink())<<info
#define JSON_LOG_IF_FIRST_N(severity, info, condition, n) KLOG_IF_FIRST_N(severity, condition, n).to_sink_only(::turbo::json_log_sink())<<info
#define JSON_LOG_IF_ONCE(severity, info, condition) KLOG_IF_ONCE(severity, condition).to_sink_only(::turbo::json_log_sink())<<info

#define JSON_LOG_IF_EVERY_POW_2(severity, info, condition) KLOG_IF_EVERY_POW_2(severity, condition).to_sink_only(::turbo::json_log_sink())<<info
#define JSON_LOG_IF_EVERY_N_SEC(severity, info, condition, n_seconds) KLOG_IF_EVERY_N_SEC(severity, condition, n_seconds).to_sink_only(::turbo::json_log_sink())<<info
#define JSON_LOG_IF_EVERY_SEC(severity, info, condition) KLOG_IF_EVERY_SEC(severity, condition).to_sink_only(::turbo::json_log_sink())<<info
#define JSON_LOG_IF_EVERY_MIN(severity, info, condition) KLOG_IF_EVERY_MIN(severity, condition).to_sink_only(::turbo::json_log_sink())<<info


#define JSON_PLOG_IF_EVERY_N(severity, info, condition, n) PKLOG_IF_EVERY_N(severity, condition, n).to_sink_only(::turbo::json_log_sink())<<info
#define JSON_PLOG_IF_FIRST_N(severity, info, condition, n) PKLOG_IF_FIRST_N(severity, condition, n).to_sink_only(::turbo::json_log_sink())<<info
#define JSON_PLOG_IF_ONCE(severity, info, condition)  PKLOG_IF_ONCE(severity, condition).to_sink_only(::turbo::json_log_sink())<<info
#define JSON_PLOG_IF_EVERY_POW_2(severity, info, condition) PKLOG_IF_EVERY_POW_2(severity, condition).to_sink_only(::turbo::json_log_sink())<<info
#define JSON_PLOG_IF_EVERY_N_SEC(severity, info, condition, n_seconds) PKLOG_IF_EVERY_N_SEC(severity, condition, n_seconds).to_sink_only(::turbo::json_log_sink())<<info
#define JSON_PLOG_IF_EVERY_SEC(severity, info, condition) PKLOG_IF_EVERY_SEC(severity, condition).to_sink_only(::turbo::json_log_sink())<<info
#define JSON_PLOG_IF_EVERY_MIN(severity, info, condition)  PKLOG_IF_EVERY_MIN(severity, condition).to_sink_only(::turbo::json_log_sink())<<info


#define JSON_DLOG_IF_EVERY_N(severity, info, condition, n) DKLOG_IF_EVERY_N(severity, condition, n).to_sink_only(::turbo::json_log_sink())<<info
#define JSON_DLOG_IF_FIRST_N(severity, info, condition, n) DKLOG_IF_FIRST_N(severity, condition, n).to_sink_only(::turbo::json_log_sink())<<info
#define JSON_DLOG_IF_ONCE(severity, info, condition) DKLOG_IF_ONCE(severity, condition).to_sink_only(::turbo::json_log_sink())<<info
#define JSON_DLOG_IF_EVERY_POW_2(severity, info, condition) DKLOG_IF_EVERY_POW_2(severity, condition).to_sink_only(::turbo::json_log_sink())<<info
#define JSON_DLOG_IF_EVERY_N_SEC(severity, info, condition, n_seconds) DKLOG_IF_EVERY_N_SEC(severity, condition, n_seconds).to_sink_only(::turbo::json_log_sink())<<info
#define JSON_DLOG_IF_EVERY_SEC(severity, info, condition) DKLOG_IF_EVERY_SEC(severity, condition).to_sink_only(::turbo::json_log_sink())<<info
#define JSON_DLOG_IF_EVERY_MIN(severity, info, condition)  DKLOG_IF_EVERY_MIN(severity, condition).to_sink_only(::turbo::json_log_sink())<<info