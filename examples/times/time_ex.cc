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
// Created by jeff on 24-5-30.
//
#include <turbo/times/time.h>
#include <iostream>

int main () {
    turbo::Time t = turbo::Time::current_time();
    std::cout << "current time is " << t << std::endl;
    std::cout<< "current nano time is " << turbo::Time::current_nanoseconds() << std::endl;
    std::cout<< "current micro time is " << turbo::Time::current_microseconds() << std::endl;
    std::cout<< "current milli time is " << turbo::Time::current_milliseconds() << std::endl;
    std::cout<< "current seconds time is " << turbo::Time::current_seconds() << std::endl;
    std::cout<< "current double micro time is " << turbo::Time::current_microseconds<double>() << std::endl;
    std::cout<< "current double milli time is " << turbo::Time::current_milliseconds<double>() << std::endl;
    std::cout<< "current double seconds time is " << turbo::Time::current_seconds<double>() << std::endl;
    turbo::CivilSecond civil = turbo::Time::to_civil_second(turbo::Time::current_time(), turbo::TimeZone::local());
    std::cout << civil.year() << std::endl;
    std::cout << civil.month() << std::endl;
    std::cout << civil.day() << std::endl;
    std::cout << civil.hour() << std::endl;
    std::cout << civil.minute() << std::endl;
    std::cout << civil.second() << std::endl;

    turbo::CivilDay d1{2025, 5,1};
    d1 -= 7;
    std::cout << d1<<std::endl;
    return 0;
}