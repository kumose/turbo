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


#include <iostream>
#include <turbo/log/log_filename.h>
#include <turbo/log/json_log.h>
#include <turbo/log/logging.h>

#include "turbo/strings/escaping.h"

int main(int argc, char **argv) {
    turbo::BaseFilename b("/root/logs/turbo.krpc.log");
    std::cout << b.basename << std::endl;
    std::cout << b.directory << std::endl;
    std::cout << b.extension << std::endl;

    turbo::BaseFilename b1("/root/logs/turbo");
    std::cout << b1.basename << std::endl;
    std::cout << b1.directory << std::endl;
    std::cout << b1.extension << std::endl;


    turbo::BaseFilename b2("logs/turbo.txt");
    std::cout << b2.basename << std::endl;
    std::cout << b2.directory << std::endl;
    std::cout << b2.extension << std::endl;

    auto p = turbo::SequentialLogFilename::make_path(10,b2);
    auto p1 = turbo::SequentialLogFilename::make_path(9,b2);
    std::cout << p << std::endl;
    turbo::SequentialLogFilename s(p, b2);
    turbo::SequentialLogFilename s1(p1, b2);
    std::cout<<s.sequence()<<std::endl;
    std::cout<<s1.sequence()<<std::endl;
    std::cout<<"s(10) < s1(9):"<<((s < s1) ? "true" : "false")<<std::endl;

    turbo::CivilDay c(2025, 5, 12);
    turbo::CivilDay c1(2025, 5, 11);
    auto d = turbo::DailyLogFilename::make_path(c, b2);
    auto d1 = turbo::DailyLogFilename::make_path(c1, b2);
    std::cout<<d<<std::endl;
    std::cout<<d1<<std::endl;
    std::cout<<"s(11) < s1(12):"<<((d < d1) ? "true" : "false")<<std::endl;

    turbo::CivilHour ch(2025, 5, 12, 1);
    turbo::CivilHour ch1(2025, 5, 11, 2);
    auto h = turbo::HourlyLogFilename::make_path(ch, b2);
    auto h1 = turbo::HourlyLogFilename::make_path(ch1, b2);
    std::cout<<h<<std::endl;
    std::cout<<h1<<std::endl;
    std::cout<<"h(12-1) < h1(11-2):"<<((h < h1) ? "true" : "false")<<std::endl;
    KLOG(INFO)<<1;

    turbo::setup_json_file_sink("example_log.json");
    turbo::AppInfo app{0,0,"krpc"};
    std::vector<std::string> categories{"krpc", "pollux", "alkaid", "kthread"};
    JSON_LOG(INFO, app)<<1111;
    auto now = turbo::Time::current_time();
    auto du = turbo::Duration::seconds(50);
    uint64_t i = 0;
    while (i < 1000) {
        turbo::AppInfo app_info{i % 10, i % 7, categories[i%4]};
        JSON_LOG_TS(INFO, app_info, (now + du * i))<<i<<" test"<<turbo::c_encode(" \"abc\"");
        i++;
    }
    return 0;
}
