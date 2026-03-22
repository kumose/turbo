//
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

#include <turbo/log/flags.h>

#include <stddef.h>

#include <algorithm>
#include <cstdlib>
#include <string>

#include <turbo/base/macros.h>
#include <turbo/base/log_severity.h>
#include <turbo/flags/flag.h>
#include <turbo/flags/marshalling.h>
#include <turbo/flags/validator.h>
#include <turbo/log/globals.h>
#include <turbo/log/internal/config.h>
#include <turbo/log/internal/vlog_config.h>
#include <turbo/strings/numbers.h>
#include <turbo/strings/string_view.h>
#include <turbo/container/flat_hash_set.h>

namespace turbo {
    namespace log_internal {
        namespace {

            void SyncLoggingFlags() {
                turbo::set_flag(&FLAGS_min_log_level, static_cast<int>(turbo::min_log_level()));
                turbo::set_flag(&FLAGS_log_with_prefix, turbo::should_prepend_log_prefix());
            }

            bool RegisterSyncLoggingFlags() {
                log_internal::SetLoggingGlobalsListener(&SyncLoggingFlags);
                return true;
            }

            TURBO_ATTRIBUTE_UNUSED const bool unused = RegisterSyncLoggingFlags();

            template<typename T>
            T GetFromEnv(const char *varname, T dflt) {
                const char *val = ::getenv(varname);
                if (val != nullptr) {
                    std::string err;
                    TURBO_INTERNAL_CHECK(turbo::parse_flag(val, &dflt, &err), err.c_str());
                }
                return dflt;
            }

            constexpr turbo::LogSeverityAtLeast StderrThresholdDefault() {
                return turbo::LogSeverityAtLeast::kError;
            }

        }  // namespace
    }  // namespace log_internal
}  // namespace turbo

static turbo::flat_hash_set<int> LogSeverityAtLeastSet = {
        static_cast<int>(turbo::LogSeverityAtLeast::kInfo),
        static_cast<int>(turbo::LogSeverityAtLeast::kWarning),
        static_cast<int>(turbo::LogSeverityAtLeast::kError),
        static_cast<int>(turbo::LogSeverityAtLeast::kFatal),
        static_cast<int>(turbo::LogSeverityAtLeast::kInfinity),
};

TURBO_FLAG(int, stderr_threshold,
           static_cast<int>(turbo::log_internal::StderrThresholdDefault()),
           "Log messages at or above this threshold level are copied to stderr.")
        .on_validate(turbo::InSetValidator<int, LogSeverityAtLeastSet>::validate)
        .on_update([]() noexcept {
            turbo::log_internal::RawSetStderrThreshold(
                    static_cast<turbo::LogSeverityAtLeast>(
                            turbo::get_flag(FLAGS_stderr_threshold)));
        });

TURBO_FLAG(int, min_log_level, static_cast<int>(turbo::LogSeverityAtLeast::kInfo),
           "Messages logged at a lower level than this don't actually "
           "get logged anywhere")
        .on_validate(turbo::InSetValidator<int, LogSeverityAtLeastSet>::validate)
        .on_update([]() noexcept {
            turbo::log_internal::RawSetMinLogLevel(
                    static_cast<turbo::LogSeverityAtLeast>(
                            turbo::get_flag(FLAGS_min_log_level)));
        });

TURBO_FLAG(std::string, backtrace_log_at, "",
           "Emit a backtrace when logging at file:linenum.")
.on_update([]() noexcept {
    const std::string backtrace_log_at =
            turbo::get_flag(FLAGS_backtrace_log_at);
    if (backtrace_log_at.empty()) {
        turbo::clear_log_backtrace_location();
        return;
    }

    const size_t last_colon = backtrace_log_at.rfind(':');
    if (last_colon == backtrace_log_at.npos) {
        turbo::clear_log_backtrace_location();
        return;
    }

    const std::string_view file =
            std::string_view(backtrace_log_at).substr(0, last_colon);
    int line;
    if (!turbo::simple_atoi(
            std::string_view(backtrace_log_at).substr(last_colon + 1),
            &line)) {
        turbo::clear_log_backtrace_location();
        return;
    }
    turbo::set_log_backtrace_location(file, line);
});

TURBO_FLAG(bool, log_with_prefix, true,
           "prepend the log prefix to the start of each log line")
.on_update([]() noexcept {
    turbo::log_internal::RawEnableLogPrefix(turbo::get_flag(FLAGS_log_with_prefix));
});

TURBO_FLAG(int, verbosity, 0,
           "Show all VKLOG(m) messages for m <= this. Overridable by --vlog_module.")
        .on_validate(turbo::GtValidator<int, 0>::validate)
        .on_update([]() noexcept {
            turbo::log_internal::UpdateGlobalVLogLevel(turbo::get_flag(FLAGS_verbosity));
        });

TURBO_FLAG(
        std::string, vlog_module, "",
        "per-module log verbosity level."
        " Argument is a comma-separated list of <module name>=<log level>."
        " <module name> is a glob pattern, matched against the filename base"
        " (that is, name ignoring .cc/.h./-inl.h)."
        " A pattern without slashes matches just the file name portion, otherwise"
        " the whole file path below the workspace root"
        " (still without .cc/.h./-inl.h) is matched."
        " ? and * in the glob pattern match any single or sequence of characters"
        " respectively including slashes."
        " <log level> overrides any value given by --verbosity.")
.on_update([]() noexcept {
    turbo::log_internal::UpdateVModule(turbo::get_flag(FLAGS_vlog_module));
});

TURBO_FLAG(std::string, log_base_filename, "",
           "The base filename for the log files. like /path/to/log_file.log");

TURBO_FLAG(int, log_check_interval_s, 60, "The interval to check the log file.");

TURBO_FLAG(bool, log_truncate, false, "Truncate the log file.");

TURBO_FLAG(int, log_max_files, 100, "The max files to keep.");

TURBO_FLAG(int, log_max_days, 7, "The max save days to keep.");

TURBO_FLAG(int, log_max_file_size, 100, "The max file size to rotate. unit is MB.");

TURBO_FLAG(int, log_type, 0, "The log type corresponding to LogSinkType."
                             " 0: console log"
                             " 1: daily log file"
                             " 2: hourly log file"
                             " 3: rotating log file");
