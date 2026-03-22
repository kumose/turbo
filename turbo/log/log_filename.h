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


#pragma once

#include <string>
#include <turbo/times/time.h>

namespace turbo {
    struct BaseFilename {
        BaseFilename(std::string_view filename);

        ~BaseFilename() = default;

        std::string basename;
        std::string extension;
        std::string directory;
    };

    class LogFilename {
    public:
        virtual ~LogFilename() = default;

        [[nodiscard]] constexpr bool valid() const { return valid_; }

    protected:
        bool valid_{false};
    };

    class SequentialLogFilename : public LogFilename {
    public:
        SequentialLogFilename(std::string_view filename, const BaseFilename &base);

        ~SequentialLogFilename() override = default;

        [[nodiscard]] constexpr int compare(const SequentialLogFilename &other) const {
            return sequence_ - other.sequence_;
        }

        [[nodiscard]] constexpr int64_t sequence() const {
            return sequence_;
        }

        static std::string make_path(int64_t seq, const BaseFilename &base);

    private:
        int64_t sequence_{-1};
    };

    inline bool operator==(const SequentialLogFilename &lhs, const SequentialLogFilename &rhs) {
        return lhs.compare(rhs) == 0;
    }

    inline bool operator!=(const SequentialLogFilename &lhs, const SequentialLogFilename &rhs) {
        return lhs.compare(rhs) != 0;
    }

    inline bool operator<(const SequentialLogFilename &lhs, const SequentialLogFilename &rhs) {
        return lhs.compare(rhs) < 0;
    }

    inline bool operator>(const SequentialLogFilename &lhs, const SequentialLogFilename &rhs) {
        return lhs.compare(rhs) > 0;
    }

    class DailyLogFilename : public LogFilename {
    public:
        DailyLogFilename(std::string_view filename, const BaseFilename &base);

        ~DailyLogFilename() override = default;

        const turbo::CivilDay &datetime() const {
            return datetime_;
        }

        static std::string make_path(const turbo::Time &stamp, const turbo::TimeZone &tz, const BaseFilename &base);

        static std::string make_path(const turbo::CivilDay &civil, const BaseFilename &base);

        [[nodiscard]] constexpr int compare(const DailyLogFilename &other) const {
            if (datetime_ < other.datetime_) {
                return -1;
            }
            if (datetime_ > other.datetime_) {
                return 1;
            }
            return 0;
        }

    private:
        turbo::CivilDay datetime_;
    };

    inline bool operator==(const DailyLogFilename &lhs, const DailyLogFilename &rhs) {
        return lhs.compare(rhs) == 0;
    }

    inline bool operator!=(const DailyLogFilename &lhs, const DailyLogFilename &rhs) {
        return lhs.compare(rhs) != 0;
    }

    inline bool operator<(const DailyLogFilename &lhs, const DailyLogFilename &rhs) {
        return lhs.compare(rhs) < 0;
    }

    inline bool operator>(const DailyLogFilename &lhs, const DailyLogFilename &rhs) {
        return lhs.compare(rhs) > 0;
    }

    class HourlyLogFilename : public LogFilename {
    public:
        HourlyLogFilename(std::string_view filename, const BaseFilename &base);

        ~HourlyLogFilename() override = default;

        const turbo::CivilHour &datetime() const {
            return datetime_;
        }

        [[nodiscard]] constexpr int compare(const HourlyLogFilename &other) const {
            if (datetime_ < other.datetime_) {
                return -1;
            }
            if (datetime_ > other.datetime_) {
                return 1;
            }
            return 0;
        }
        static std::string make_path(const turbo::Time &stamp, const turbo::TimeZone &tz, const BaseFilename &base);
        static std::string make_path(const turbo::CivilHour &civil, const BaseFilename &base);

    private:
        turbo::CivilHour datetime_;
    };
} // namespace turbo
