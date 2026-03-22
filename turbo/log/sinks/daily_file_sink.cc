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

#include <turbo/log/sinks/daily_file_sink.h>
#include <turbo/times/clock.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <thread>
#include <turbo/log/internal/globals.h>
#include <turbo/strings/str_format.h>
#include <turbo/log/internal/fs_helper.h>

namespace turbo {

    DailyFileSink::DailyFileSink(std::string_view base_filename,
                                 uint16_t max_files,
                                 int check_interval_s,
                                 bool truncate)
        : _raw_base_filename(base_filename),
          _base_filename(base_filename),
          _truncate(truncate),
          _max_files(max_files),
          _check_interval_s(check_interval_s),
          _next_check_time(turbo::Time::current_time() + turbo::Duration::seconds(check_interval_s)) {
        _next_rotation_time = next_rotation_time(turbo::Time::current_time());
        if (_max_files > 0) {
            init_file_queue();
        }
        auto now = turbo::Time::current_time();
        auto filename = DailyLogFilename::make_path(now, *turbo::log_internal::TimeZone(), _base_filename);
        _file_writer = std::make_unique<log_internal::AppendFile>();
        if (_truncate) {
            ::remove(filename.c_str());
        }
        rotate_file(now);
    }

    DailyFileSink::~DailyFileSink() {
        if (_file_writer != nullptr) {
            _file_writer->close();
        }
    }

    void DailyFileSink::Send(const LogEntry &entry) {
        std::unique_lock lock(_mutex);
        rotate_file(entry.timestamp());
        if (_file_writer == nullptr) {
            return;
        }
        // Write to the current file
        auto logdata = entry.newline()
                           ? entry.text_message_with_prefix_and_newline()
                           : entry.text_message_with_prefix();
        if (TURBO_LIKELY(entry.log_severity() != LogSeverity::kFatal)) {
            _file_writer->write(logdata);
        } else {
            if (!entry.stacktrace().empty()) {
                _file_writer->write(logdata);
                _file_writer->write(entry.stacktrace());
            }
        }
    }

    void DailyFileSink::init_file_queue() {
        std::vector<std::string> filenames;
        _files = circular_queue<std::string>(static_cast<size_t>(_max_files));
        auto now = turbo::Time::current_time();
        while (filenames.size() < _max_files) {
            auto filename = DailyLogFilename::make_path(now, *turbo::log_internal::TimeZone(), _base_filename);
            if (!log_internal::path_exists(filename)) {
                break;
            }
            filenames.emplace_back(filename);
            now -= turbo::Duration::hours(24);
        }
        for (auto iter = filenames.rbegin(); iter != filenames.rend(); ++iter) {
            _files.push_back(std::move(*iter));
        }
    }

    void DailyFileSink::rotate_file(turbo::Time stamp) {
        if (stamp >= _next_check_time) {
            _next_check_time = stamp + turbo::Duration::seconds(_check_interval_s);
            if (_file_writer != nullptr)
                _file_writer->reopen();
        }
        if (stamp < _next_rotation_time) {
            return;
        }
        _next_rotation_time = next_rotation_time(stamp);
        auto filename = DailyLogFilename::make_path(stamp, *turbo::log_internal::TimeZone(), _base_filename);
        _file_writer->close();
        _file_writer.reset();
        _file_writer = std::make_unique<log_internal::AppendFile>();
        _file_writer->initialize(filename);
        if (_max_files == 0) {
            return;
        }

        auto current_file = _file_writer->file_path();
        if (_files.full()) {
            auto old_filename = std::move(_files.front());
            _files.pop_front();
            bool ok = log_internal::remove_if_exists(old_filename) == 0;
            if (!ok) {
                std::cerr << "Failed removing daily file " + old_filename << std::endl;
            }
        }
        _files.push_back(std::move(current_file));
    }

    turbo::Time DailyFileSink::next_rotation_time(turbo::Time stamp) const {
        auto tm = turbo::Time::to_tm(stamp, turbo::TimeZone::local());
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;
        auto rotation_time = turbo::Time::from_tm(tm, turbo::TimeZone::local());
        if (rotation_time > stamp) {
            return rotation_time;
        }
        return rotation_time + turbo::Duration::hours(24);
    }

    void DailyFileSink::Flush() {
        std::unique_lock lock(_mutex);
        // Close the current file
        // Rotate the files
        if (_file_writer != nullptr) {
            _file_writer->flush();
        }
    }
} // namespace  turbo
