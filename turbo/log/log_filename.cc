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



#include <turbo/log/log_filename.h>
#include <turbo/files/filesystem.h>
#include <turbo/strings/match.h>
#include <turbo/strings/numbers.h>
#include <turbo/strings/str_format.h>
#include <turbo/strings/str_split.h>

namespace turbo {
    BaseFilename::BaseFilename(std::string_view filename) {
        const turbo::FilePath path(filename);
        directory = path.parent_path();
        basename = path.filename().filename().stem();
        extension = path.extension();
    }

    SequentialLogFilename::SequentialLogFilename(std::string_view filename, const BaseFilename &base) {
        const turbo::FilePath path(filename);
        const std::string basename = path.filename().filename().stem();
        const std::string extension = path.extension();
        const std::string directory = path.parent_path().string();
        if (extension != base.extension) {
            LogFilename::valid_ = false;
            return;
        }
        if (!turbo::starts_with(basename, base.basename)) {
            LogFilename::valid_ = false;
            return;
        }
        std::string_view seq = basename;
        seq = seq.substr(base.basename.size());
        if (seq.empty() || seq.front() != '_') {
            LogFilename::valid_ = false;
            return;
        }
        seq = seq.substr(1);
        if (seq.empty()) {
            LogFilename::valid_ = false;
        }
        if (!turbo::simple_atoi(seq, &sequence_)) {
            LogFilename::valid_ = false;
            return;
        }
    }

    std::string SequentialLogFilename::make_path(int64_t seq, const BaseFilename &base) {
        if (base.directory.empty()) {
            return turbo::str_format("%s_%04d%s", base.basename, seq, base.extension);
        } else {
            return turbo::str_format("%s/%s_%04d%s", base.directory, base.basename, seq, base.extension);
        }
    }

    DailyLogFilename::DailyLogFilename(std::string_view filename, const BaseFilename &base) {
        const turbo::FilePath path(filename);
        const std::string basename = path.filename().filename().stem();
        const std::string extension = path.extension();
        const std::string directory = path.parent_path().string();
        if (extension != base.extension) {
            LogFilename::valid_ = false;
            return;
        }
        if (!turbo::starts_with(basename, base.basename)) {
            LogFilename::valid_ = false;
            return;
        }
        std::string_view seq = basename;
        seq = seq.substr(base.basename.size());
        if (seq.empty() || seq.front() != '_') {
            LogFilename::valid_ = false;
            return;
        }
        seq = seq.substr(1);
        if (seq.empty()) {
            LogFilename::valid_ = false;
        }
        if (!turbo::parse_civil_time(seq, &datetime_)) {
            LogFilename::valid_ = false;
            return;
        }
    }

    std::string DailyLogFilename::make_path(const turbo::Time &stamp, const turbo::TimeZone &tz, const BaseFilename &base) {
        const turbo::CivilDay civil_day = turbo::Time::to_civil_day(stamp, tz);
        return make_path(civil_day, base);
    }
    std::string DailyLogFilename::make_path(const turbo::CivilDay &civil, const BaseFilename &base) {
        if (base.directory.empty()) {
            return turbo::str_format("%s_%v%s", base.basename, civil, base.extension);
        } else {
            return turbo::str_format("%s/%s_%v%s", base.directory, base.basename, civil, base.extension);
        }
    }

    HourlyLogFilename::HourlyLogFilename(std::string_view filename, const BaseFilename &base) {
        const turbo::FilePath path(filename);
        const std::string basename = path.filename().filename().stem();
        const std::string extension = path.extension();
        const std::string directory = path.parent_path().string();
        if (extension != base.extension) {
            LogFilename::valid_ = false;
            return;
        }
        if (!turbo::starts_with(basename, base.basename)) {
            LogFilename::valid_ = false;
            return;
        }
        std::string_view seq = basename;
        seq = seq.substr(base.basename.size());
        if (seq.empty() || seq.front() != '_') {
            LogFilename::valid_ = false;
            return;
        }
        seq = seq.substr(1);
        if (seq.empty()) {
            LogFilename::valid_ = false;
        }
        if (!turbo::parse_civil_time(seq, &datetime_)) {
            LogFilename::valid_ = false;
            return;
        }
    }

    std::string HourlyLogFilename::make_path(const turbo::Time &stamp, const turbo::TimeZone &tz, const BaseFilename &base) {
        const turbo::CivilHour civil_hour = turbo::Time::to_civil_hour(stamp, tz);
        return make_path(civil_hour, base);
    }

    std::string HourlyLogFilename::make_path(const turbo::CivilHour &civil, const BaseFilename &base) {
        if (base.directory.empty()) {
            return turbo::str_format("%s_%v%s", base.basename, civil, base.extension);
        } else {
            return turbo::str_format("%s/%s_%v%s", base.directory, base.basename, civil, base.extension);
        }
    }
} // namespace turbo
