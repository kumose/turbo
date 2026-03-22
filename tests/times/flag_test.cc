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

#include <turbo/flags/flag.h>

#include <string>

#include <gtest/gtest.h>
#include <turbo/flags/reflection.h>
#include <turbo/times/civil_time.h>
#include <turbo/times/time.h>

TURBO_FLAG(turbo::CivilSecond, test_flag_civil_second,
           turbo::CivilSecond(2015, 1, 2, 3, 4, 5), "");
TURBO_FLAG(turbo::CivilMinute, test_flag_civil_minute,
           turbo::CivilMinute(2015, 1, 2, 3, 4), "");
TURBO_FLAG(turbo::CivilHour, test_flag_civil_hour, turbo::CivilHour(2015, 1, 2, 3),
           "");
TURBO_FLAG(turbo::CivilDay, test_flag_civil_day, turbo::CivilDay(2015, 1, 2), "");
TURBO_FLAG(turbo::CivilMonth, test_flag_civil_month, turbo::CivilMonth(2015, 1),
           "");
TURBO_FLAG(turbo::CivilYear, test_flag_civil_year, turbo::CivilYear(2015), "");

TURBO_FLAG(turbo::Duration, test_duration_flag, turbo::Duration::seconds(5),
           "For testing support for Duration flags");
TURBO_FLAG(turbo::Time, test_time_flag, turbo::Time::past_infinite(),
           "For testing support for Time flags");

namespace {

    bool SetFlagValue(std::string_view flag_name, std::string_view value) {
        auto *flag = turbo::find_command_line_flag(flag_name);
        if (!flag) return false;
        std::string err;
        return flag->parse_from(value, &err);
    }

    bool GetFlagValue(std::string_view flag_name, std::string &value) {
        auto *flag = turbo::find_command_line_flag(flag_name);
        if (!flag) return false;
        value = flag->current_value();
        return true;
    }

    TEST(CivilTime, FlagSupport) {
        // Tests the default setting of the flags.
        const turbo::CivilSecond kDefaultSec(2015, 1, 2, 3, 4, 5);
        EXPECT_EQ(turbo::CivilSecond(kDefaultSec),
                  turbo::get_flag(FLAGS_test_flag_civil_second));
        EXPECT_EQ(turbo::CivilMinute(kDefaultSec),
                  turbo::get_flag(FLAGS_test_flag_civil_minute));
        EXPECT_EQ(turbo::CivilHour(kDefaultSec),
                  turbo::get_flag(FLAGS_test_flag_civil_hour));
        EXPECT_EQ(turbo::CivilDay(kDefaultSec),
                  turbo::get_flag(FLAGS_test_flag_civil_day));
        EXPECT_EQ(turbo::CivilMonth(kDefaultSec),
                  turbo::get_flag(FLAGS_test_flag_civil_month));
        EXPECT_EQ(turbo::CivilYear(kDefaultSec),
                  turbo::get_flag(FLAGS_test_flag_civil_year));

        // Sets flags to a new value.
        const turbo::CivilSecond kNewSec(2016, 6, 7, 8, 9, 10);
        turbo::set_flag(&FLAGS_test_flag_civil_second, turbo::CivilSecond(kNewSec));
        turbo::set_flag(&FLAGS_test_flag_civil_minute, turbo::CivilMinute(kNewSec));
        turbo::set_flag(&FLAGS_test_flag_civil_hour, turbo::CivilHour(kNewSec));
        turbo::set_flag(&FLAGS_test_flag_civil_day, turbo::CivilDay(kNewSec));
        turbo::set_flag(&FLAGS_test_flag_civil_month, turbo::CivilMonth(kNewSec));
        turbo::set_flag(&FLAGS_test_flag_civil_year, turbo::CivilYear(kNewSec));

        EXPECT_EQ(turbo::CivilSecond(kNewSec),
                  turbo::get_flag(FLAGS_test_flag_civil_second));
        EXPECT_EQ(turbo::CivilMinute(kNewSec),
                  turbo::get_flag(FLAGS_test_flag_civil_minute));
        EXPECT_EQ(turbo::CivilHour(kNewSec),
                  turbo::get_flag(FLAGS_test_flag_civil_hour));
        EXPECT_EQ(turbo::CivilDay(kNewSec), turbo::get_flag(FLAGS_test_flag_civil_day));
        EXPECT_EQ(turbo::CivilMonth(kNewSec),
                  turbo::get_flag(FLAGS_test_flag_civil_month));
        EXPECT_EQ(turbo::CivilYear(kNewSec),
                  turbo::get_flag(FLAGS_test_flag_civil_year));
    }

    TEST(Duration, FlagSupport) {
        EXPECT_EQ(turbo::Duration::seconds(5), turbo::get_flag(FLAGS_test_duration_flag));

        turbo::set_flag(&FLAGS_test_duration_flag, turbo::Duration::seconds(10));
        EXPECT_EQ(turbo::Duration::seconds(10), turbo::get_flag(FLAGS_test_duration_flag));

        EXPECT_TRUE(SetFlagValue("test_duration_flag", "20s"));
        EXPECT_EQ(turbo::Duration::seconds(20), turbo::get_flag(FLAGS_test_duration_flag));

        std::string current_flag_value;
        EXPECT_TRUE(GetFlagValue("test_duration_flag", current_flag_value));
        EXPECT_EQ("20s", current_flag_value);
    }

    TEST(Time, FlagSupport) {
        EXPECT_EQ(turbo::Time::past_infinite(), turbo::get_flag(FLAGS_test_time_flag));

        const turbo::Time t = turbo::Time::from_civil(turbo::CivilSecond(2016, 1, 2, 3, 4, 5),
                                               turbo::TimeZone::utc());
        turbo::set_flag(&FLAGS_test_time_flag, t);
        EXPECT_EQ(t, turbo::get_flag(FLAGS_test_time_flag));

        // Successful parse
        EXPECT_TRUE(SetFlagValue("test_time_flag", "2016-01-02T03:04:06Z"));
        EXPECT_EQ(t + turbo::Duration::seconds(1), turbo::get_flag(FLAGS_test_time_flag));
        EXPECT_TRUE(SetFlagValue("test_time_flag", "2016-01-02T03:04:07.0Z"));
        EXPECT_EQ(t + turbo::Duration::seconds(2), turbo::get_flag(FLAGS_test_time_flag));
        EXPECT_TRUE(SetFlagValue("test_time_flag", "2016-01-02T03:04:08.000Z"));
        EXPECT_EQ(t + turbo::Duration::seconds(3), turbo::get_flag(FLAGS_test_time_flag));
        EXPECT_TRUE(SetFlagValue("test_time_flag", "2016-01-02T03:04:09+00:00"));
        EXPECT_EQ(t + turbo::Duration::seconds(4), turbo::get_flag(FLAGS_test_time_flag));
        EXPECT_TRUE(SetFlagValue("test_time_flag", "2016-01-02T03:04:05.123+00:00"));
        EXPECT_EQ(t + turbo::Duration::milliseconds(123), turbo::get_flag(FLAGS_test_time_flag));
        EXPECT_TRUE(SetFlagValue("test_time_flag", "2016-01-02T03:04:05.123+08:00"));
        EXPECT_EQ(t + turbo::Duration::milliseconds(123) - turbo::Duration::hours(8),
                  turbo::get_flag(FLAGS_test_time_flag));
        EXPECT_TRUE(SetFlagValue("test_time_flag", "infinite-future"));
        EXPECT_EQ(turbo::Time::future_infinite(), turbo::get_flag(FLAGS_test_time_flag));
        EXPECT_TRUE(SetFlagValue("test_time_flag", "infinite-past"));
        EXPECT_EQ(turbo::Time::past_infinite(), turbo::get_flag(FLAGS_test_time_flag));

        EXPECT_FALSE(SetFlagValue("test_time_flag", "2016-01-02T03:04:06"));
        EXPECT_FALSE(SetFlagValue("test_time_flag", "2016-01-02"));
        EXPECT_FALSE(SetFlagValue("test_time_flag", "2016-01-02Z"));
        EXPECT_FALSE(SetFlagValue("test_time_flag", "2016-01-02+00:00"));
        EXPECT_FALSE(SetFlagValue("test_time_flag", "2016-99-99T03:04:06Z"));

        EXPECT_TRUE(SetFlagValue("test_time_flag", "2016-01-02T03:04:05Z"));
        std::string current_flag_value;
        EXPECT_TRUE(GetFlagValue("test_time_flag", current_flag_value));
        EXPECT_EQ("2016-01-02T03:04:05+00:00", current_flag_value);
    }

}  // namespace
