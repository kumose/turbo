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

#include <turbo/log/logging.h>
#include <turbo/log/log_sink_registry.h>
#include <turbo/log/initialize.h>
#include <turbo/log/sinks/ansicolor_sink.h>
#include <turbo/log/sinks/daily_file_sink.h>
#include <turbo/log/sinks/hourly_file_sink.h>
#include <turbo/log/sinks/json_file_sink.h>
#include <turbo/log/sinks/rotating_file_sink.h>
#include <turbo/log/flags.h>
#include <turbo/flags/flag.h>
#include <turbo/strings/str_split.h>
#include <iostream>

namespace turbo {
    std::mutex g_sink_mutex;
    std::shared_ptr<turbo::LogSink> g_sink;
    std::shared_ptr<turbo::LogSink> json_sink;

    std::string g_multi_register_die_message = "Sink already registered you can only register one sink"
            "if you want to register multiple sinks please use the log_sink_registry.h, "
            "and call add_log_sink() to add the sink to the registry";

    void setup_daily_file_sink(const std::string &base_filename,
                               uint16_t max_files,
                               int check_interval_s,
                               bool truncate) {
        std::unique_lock lock(g_sink_mutex);
        if (g_sink != nullptr) {
            std::cerr << g_multi_register_die_message << std::endl;
            return;
        }

        g_sink = std::make_shared<DailyFileSink>(base_filename, max_files, check_interval_s,
                                                 truncate);
        initialize_log();
        add_log_sink(g_sink.get());
    }

    void setup_hourly_file_sink(std::string_view base_filename,
                                uint16_t max_files,
                                int check_interval_s,
                                bool truncate) {
        std::unique_lock lock(g_sink_mutex);
        if (g_sink != nullptr) {
            std::cerr << g_multi_register_die_message << std::endl;
            return;
        }

        g_sink = std::make_shared<HourlyFileSink>(base_filename, max_files, check_interval_s, truncate);
        initialize_log();
        add_log_sink(g_sink.get());
    }

    void setup_json_file_sink(std::string_view base_filename,
                              uint16_t max_files,
                              int check_interval_s,
                              bool truncate) {
        std::unique_lock lock(g_sink_mutex);
        if (g_sink != nullptr) {
            std::cerr << g_multi_register_die_message << std::endl;
            return;
        }

        json_sink = std::make_shared<JsonFileSink>(base_filename, max_files, check_interval_s, truncate);
        set_global_json_sink(json_sink.get());
        initialize_log();
    }

    void setup_rotating_file_sink(const std::string &base_filename,
                                  int max_file_size_mb,
                                  uint16_t max_files,
                                  bool truncate,
                                  int check_interval_s) {
        std::unique_lock lock(g_sink_mutex);
        if (g_sink != nullptr) {
            std::cerr << g_multi_register_die_message << std::endl;
            return;
        }
        static const int one_mb = 1024 * 1024;
        g_sink = std::make_shared<RotatingFileSink>(base_filename, max_file_size_mb * one_mb, check_interval_s,
                                                    max_files);
        initialize_log();
        add_log_sink(g_sink.get());
    }

    void setup_ansi_color_stdout_sink() {
        std::unique_lock lock(g_sink_mutex);
        if (g_sink != nullptr) {
            std::cerr << g_multi_register_die_message << std::endl;
            return;
        }

        g_sink = std::make_shared<AnsiColorSink>(stdout);
        initialize_log();
        set_stderr_threshold(LogSeverityAtLeast::kInfinity);
        add_log_sink(g_sink.get());
    }

    void setup_color_stderr_sink() {
        std::unique_lock lock(g_sink_mutex);
        if (g_sink != nullptr) {
            std::cerr << g_multi_register_die_message << std::endl;
            return;
        }

        g_sink = std::make_shared<AnsiColorSink>(stderr);
        initialize_log();
        set_stderr_threshold(LogSeverityAtLeast::kInfinity);
        add_log_sink(g_sink.get());
    }

    void cleanup_log() {
        std::unique_lock lock(g_sink_mutex);
        if (g_sink != nullptr) {
            remove_log_sink(g_sink.get());
            g_sink = nullptr;
        }
    }

    void enable_stderr_logging(LogSeverityAtLeast threshold) {
        initialize_log();
        set_stderr_threshold(threshold);
    }

    void disable_stderr_logging() {
        initialize_log();
        set_stderr_threshold(LogSeverityAtLeast::kInfinity);
    }

    void load_flags_symbol() {
        (void) turbo::get_flag(FLAGS_stderr_threshold);
        (void) turbo::get_flag(FLAGS_min_log_level);
        (void) turbo::get_flag(FLAGS_backtrace_log_at);
        (void) turbo::get_flag(FLAGS_log_with_prefix);
        (void) turbo::get_flag(FLAGS_verbosity);
        (void) turbo::get_flag(FLAGS_vlog_module);
    }

    void setup_log_by_flags() {
        auto lt = static_cast<LogSinkType>(turbo::get_flag(FLAGS_log_type));
        switch (lt) {
            case LogSinkType::kColorStderr:
                setup_color_stderr_sink();
                break;
            case LogSinkType::kDailyFile:
                setup_daily_file_sink(turbo::get_flag(FLAGS_log_base_filename),
                                      turbo::get_flag(FLAGS_log_max_days),
                                      turbo::get_flag(FLAGS_log_check_interval_s),
                                      turbo::get_flag(FLAGS_log_truncate));
                break;
            case LogSinkType::kHourlyFile:
                setup_hourly_file_sink(turbo::get_flag(FLAGS_log_base_filename),
                                       turbo::get_flag(FLAGS_log_max_days) * 24,
                                       turbo::get_flag(FLAGS_log_check_interval_s),
                                       turbo::get_flag(FLAGS_log_truncate)
                );
                break;
            case LogSinkType::kRotatingFile:
                setup_rotating_file_sink(turbo::get_flag(FLAGS_log_base_filename),
                                         turbo::get_flag(FLAGS_log_max_file_size),
                                         turbo::get_flag(FLAGS_log_max_files),
                                         turbo::get_flag(FLAGS_log_truncate),
                                         turbo::get_flag(FLAGS_log_check_interval_s));
                break;
            default:
                setup_color_stderr_sink();
                break;
        }
    }

    std::string make_default_log_filename(std::string_view argv0) {
        std::vector<std::string_view> sps = turbo::str_split(argv0, "/");
        return turbo::str_format("logs/%s_log.txt", sps.back());
    }
} // namespace turbo
