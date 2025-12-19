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
//
// -----------------------------------------------------------------------------
// File: log/log.h
// -----------------------------------------------------------------------------
//
// This header declares a family of KLOG macros.
//
// Basic invocation looks like this:
//
//   KLOG(INFO) << "Found " << num_cookies << " cookies";
//
// Most `KLOG` macros take a severity level argument.  The severity levels are
// `INFO`, `WARNING`, `ERROR`, and `FATAL`.  They are defined
// in turbo/base/log_severity.h.
// * The `FATAL` severity level terminates the program with a stack trace after
//   logging its message.  Error handlers registered with `RunOnFailure`
//   (process_state.h) are run, but exit handlers registered with `atexit(3)`
//   are not.
// * The `QFATAL` pseudo-severity level is equivalent to `FATAL` but triggers
//   quieter termination messages, e.g. without a full stack trace, and skips
//   running registered error handlers.
// * The `DFATAL` pseudo-severity level is defined as `FATAL` in debug mode and
//   as `ERROR` otherwise.
// Some preprocessor shenanigans are used to ensure that e.g. `KLOG(INFO)` has
// the same meaning even if a local symbol or preprocessor macro named `INFO` is
// defined.  To specify a severity level using an expression instead of a
// literal, use `LEVEL(expr)`.
// Example:
//
//   KLOG(LEVEL(stale ? turbo::LogSeverity::kWarning : turbo::LogSeverity::kInfo))
//       << "Cookies are " << days << " days old";

// `KLOG` macros evaluate to an unterminated statement.  The value at the end of
// the statement supports some chainable methods:
//
//   * .at_location(std::string_view file, int line)
//     .at_location(turbo::SourceLocation loc)
//     Overrides the location inferred from the callsite.  The string pointed to
//     by `file` must be valid until the end of the statement.
//   * .no_prefix()
//     Omits the prefix from this line.  The prefix includes metadata about the
//     logged data such as source code location and timestamp.
//   * .with_verbosity(int verbose_level)
//     Sets the verbosity field of the logged message as if it was logged by
//     `VKLOG(verbose_level)`.  Unlike `VKLOG`, this method does not affect
//     evaluation of the statement when the specified `verbose_level` has been
//     disabled.  The only effect is on `LogSink` implementations which make use
//     of the `turbo::LogSink::verbosity()` value.  The value
//     `turbo::LogEntry::kNoVerbosityLevel` can be specified to mark the message
//     not verbose.
//   * .with_timestamp(turbo::Time timestamp)
//     Uses the specified timestamp instead of one collected at the time of
//     execution.
//   * .with_thread_id(turbo::LogEntry::tid_t tid)
//     Uses the specified thread ID instead of one collected at the time of
//     execution.
//   * .with_metadata_from(const turbo::LogEntry &entry)
//     Copies all metadata (but no data) from the specified `turbo::LogEntry`.
//     This can be used to change the severity of a message, but it has some
//     limitations:
//     * `TURBO_MIN_LOG_LEVEL` is evaluated against the severity passed into
//       `KLOG` (or the implicit `FATAL` level of `KCHECK`).
//     * `KLOG(FATAL)` and `KCHECK` terminate the process unconditionally, even if
//       the severity is changed later.
//     `.with_metadata_from(entry)` should almost always be used in combination
//     with `KLOG(LEVEL(entry.log_severity()))`.
//   * .with_perror()
//     Appends to the logged message a colon, a space, a textual description of
//     the current value of `errno` (as by `strerror(3)`), and the numerical
//     value of `errno`.
//   * .to_sink_also(turbo::LogSink* sink)
//     Sends this message to `*sink` in addition to whatever other sinks it
//     would otherwise have been sent to.  `sink` must not be null.
//   * .to_sink_only(turbo::LogSink* sink)
//     Sends this message to `*sink` and no others.  `sink` must not be null.
//
// No interfaces in this header are async-signal-safe; their use in signal
// handlers is unsupported and may deadlock your program or eat your lunch.
//
// Many logging statements are inherently conditional.  For example,
// `KLOG_IF(INFO, !foo)` does nothing if `foo` is true.  Even seemingly
// unconditional statements like `KLOG(INFO)` might be disabled at
// compile-time to minimize binary size or for security reasons.
//
// * Except for the condition in a `KCHECK` or `QKCHECK` statement, programs must
//   not rely on evaluation of expressions anywhere in logging statements for
//   correctness.  For example, this is ok:
//
//     KCHECK((fp = fopen("config.ini", "r")) != nullptr);
//
//   But this is probably not ok:
//
//     KLOG(INFO) << "Server status: " << StartServerAndReturnStatusString();
//
//   The example below is bad too; the `i++` in the `KLOG_IF` condition might
//   not be evaluated, resulting in an infinite loop:
//
//     for (int i = 0; i < 1000000;)
//       KLOG_IF(INFO, i++ % 1000 == 0) << "Still working...";
//
// * Except where otherwise noted, conditions which cause a statement not to log
//   also cause expressions not to be evaluated.  Programs may rely on this for
//   performance reasons, e.g. by streaming the result of an expensive function
//   call into a `DKLOG` or `KLOG_EVERY_N` statement.
// * Care has been taken to ensure that expressions are parsed by the compiler
//   even if they are never evaluated.  This means that syntax errors will be
//   caught and variables will be considered used for the purposes of
//   unused-variable diagnostics.  For example, this statement won't compile
//   even if `INFO`-level logging has been compiled out:
//
//     int number_of_cakes = 40;
//     KLOG(INFO) << "Number of cakes: " << number_of_cake;  // Note the typo!
//
//   Similarly, this won't produce unused-variable compiler diagnostics even
//   if `INFO`-level logging is compiled out:
//
//     {
//       char fox_line1[] = "Hatee-hatee-hatee-ho!";
//       KLOG_IF(ERROR, false) << "The fox says " << fox_line1;
//       char fox_line2[] = "A-oo-oo-oo-ooo!";
//       KLOG(INFO) << "The fox also says " << fox_line2;
//     }
//
//   This error-checking is not perfect; for example, symbols that have been
//   declared but not defined may not produce link errors if used in logging
//   statements that compile away.
//
// Expressions streamed into these macros are formatted using `operator<<` just
// as they would be if streamed into a `std::ostream`, however it should be
// noted that their actual type is unspecified.
//
// To implement a custom formatting operator for a type you own, there are two
// options: `turbo_stringify()` or `std::ostream& operator<<(std::ostream&, ...)`.
// It is recommended that users make their types loggable through
// `turbo_stringify()` as it is a universal stringification extension that also
// enables `turbo::str_format` and `turbo::str_cat` support. If both
// `turbo_stringify()` and `std::ostream& operator<<(std::ostream&, ...)` are
// defined, `turbo_stringify()` will be used.
//
// To use the `turbo_stringify()` API, define a friend function template in your
// type's namespace with the following signature:
//
//   template <typename Sink>
//   void turbo_stringify(Sink& sink, const UserDefinedType& value);
//
// `Sink` has the same interface as `turbo::FormatSink`, but without
// `PutPaddedString()`.
//
// Example:
//
//   struct Point {
//     template <typename Sink>
//     friend void turbo_stringify(Sink& sink, const Point& p) {
//       turbo::format(&sink, "(%v, %v)", p.x, p.y);
//     }
//
//     int x;
//     int y;
//   };
//
// To use `std::ostream& operator<<(std::ostream&, ...)`, define
// `std::ostream& operator<<(std::ostream&, ...)` in your type's namespace (for
// ADL) just as you would to stream it to `std::cout`.
//
// Currently `turbo_stringify()` ignores output manipulators but this is not
// guaranteed behavior and may be subject to change in the future. If you would
// like guaranteed behavior regarding output manipulators, please use
// `std::ostream& operator<<(std::ostream&, ...)` to make custom types loggable
// instead.
//
// Those macros that support streaming honor output manipulators and `fmtflag`
// changes that output data (e.g. `std::ends`) or control formatting of data
// (e.g. `std::hex` and `std::fixed`), however flushing such a stream is
// ignored.  The message produced by a log statement is sent to registered
// `turbo::LogSink` instances at the end of the statement; those sinks are
// responsible for their own flushing (e.g. to disk) semantics.
//
// Flag settings are not carried over from one `KLOG` statement to the next; this
// is a bit different than e.g. `std::cout`:
//
//   KLOG(INFO) << std::hex << 0xdeadbeef;  // logs "0xdeadbeef"
//   KLOG(INFO) << 0xdeadbeef;              // logs "3735928559"

#pragma once

#include <turbo/log/klog.h>
#include <turbo/log/json_log.h>
