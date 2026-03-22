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
// Created by jeff on 24-6-28.
//

#include <turbo/flags/flag.h>
#include <turbo/flags/reflection.h>
#include <iostream>

turbo::flat_hash_set<int> set{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

TURBO_FLAG(std::string, test_flag, "test", "test flag").on_validate(turbo::AllPassValidator<std::string>::validate);

TURBO_FLAG(int, gt_flag, 10, "test flag").on_validate(turbo::GeValidator<int, 5>::validate);

TURBO_FLAG(int, range_flag, 10, "test flag").on_validate(turbo::ClosedOpenInRangeValidator<int, 5,15>::validate);

TURBO_FLAG(int, inset_flag, 3, "test flag").on_validate(turbo::InSetValidator<int, set>::validate);

std::string_view prefix = "/opt/EA";
TURBO_FLAG(std::string, prefix_flag, "/opt/EA/inf", "test flag").on_validate(turbo::StartsWithValidator<prefix>::validate);

int main(int argc, char **argv) {
    std::cout << "test_flag: " << turbo::get_flag(FLAGS_test_flag) << std::endl;
    turbo::set_flag(&FLAGS_test_flag, "test2");
    std::cout << "test_flag: " << turbo::get_flag(FLAGS_test_flag) << std::endl;
    auto flag = turbo::find_command_line_flag("test_flag");
    if (flag) {
        std::cout << "flag: " << flag->name() << std::endl;
        if(flag->has_user_validator()) {
            std::cout << "flag has user validator" << std::endl;
            std::cout<<flag->user_validate("test3", nullptr)<<std::endl;
        }

    }

    auto gt_flag = turbo::find_command_line_flag("gt_flag");
    std::cout<<"this should be 0, "<<gt_flag->user_validate("4", nullptr)<<std::endl;
    std::cout<<"this should be 1, "<<gt_flag->user_validate("6", nullptr)<<std::endl;

    auto inset_flag = turbo::find_command_line_flag("inset_flag");
    std::cout<<"this should be 0, "<<inset_flag->user_validate("11", nullptr)<<std::endl;
    std::cout<<"this should be 1, "<<inset_flag->user_validate("7", nullptr)<<std::endl;

    auto prefix_flag = turbo::find_command_line_flag("prefix_flag");
    std::cout<<"this should be 0, "<<prefix_flag->user_validate("/opt/ea", nullptr)<<std::endl;
    std::cout<<"this should be 1, "<<prefix_flag->user_validate("/opt/EA/inf", nullptr)<<std::endl;

    auto range_flag = turbo::find_command_line_flag("range_flag");
    std::cout<<"this should be 0, "<<range_flag->user_validate("4", nullptr)<<std::endl;
    std::cout<<"this should be 0, "<<range_flag->user_validate("15", nullptr)<<std::endl;
    std::cout<<"this should be 0, "<<range_flag->user_validate("20", nullptr)<<std::endl;
    std::cout<<"this should be 0, "<<range_flag->user_validate("a", nullptr)<<std::endl;
    std::cout<<"this should be 1, "<<range_flag->user_validate("6", nullptr)<<std::endl;
    std::cout<<"this should be 1, "<<range_flag->user_validate("5", nullptr)<<std::endl;

}