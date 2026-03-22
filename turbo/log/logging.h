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
//
#pragma once

#include <turbo/log/klog.h>
#include <turbo/log/check.h>
#include <turbo/log/die_if_null.h>
#include <turbo/log/vlog_is_on.h>
#include <turbo/log/globals.h>
#include <turbo/log/initialize.h>

namespace turbo {
    enum class LogSinkType {
        kColorStderr,
        kDailyFile,
        kHourlyFile,
        kRotatingFile,
    };

    // verbose log level for all
    static constexpr int V_ALL = 0;

    // verbose log level for important information
    static constexpr int V_IMPORTANT = 100;

    // verbose log level for debug information
    static constexpr int V_DEBUG = 200;

    // verbose log level for trace information
    static constexpr int V_TRACE = 300;

    void setup_daily_file_sink(const std::string &base_filename,
                               uint16_t max_files = 7,
                               int check_interval_s = 60,
                               bool truncate = false
    );

    void setup_hourly_file_sink(std::string_view base_filename,
                                uint16_t max_files = 84,
                                int check_interval_s = 60,
                                bool truncate = false
    );

    void setup_json_file_sink(std::string_view base_filename,
                              uint16_t max_files = 84,
                              int check_interval_s = 60,
                              bool truncate = false);

    void setup_rotating_file_sink(const std::string &base_filename,
                                  int max_file_size_mb = 100,
                                  uint16_t max_files = 100,
                                  bool truncate = false,
                                  int check_interval_s = 60);

    void setup_ansi_color_stdout_sink();

    void setup_color_stderr_sink();

    void enable_stderr_logging(LogSeverityAtLeast threshold = LogSeverityAtLeast::kError);

    void disable_stderr_logging();

    void cleanup_log();

    void load_flags_symbol();

    void setup_log_by_flags();

    std::string make_default_log_filename(std::string_view argv0);
} // namespace turbo
