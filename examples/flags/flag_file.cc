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
#include <turbo/flags/app.h>
#include <iostream>
#include <turbo/flags/declare.h>
#include <turbo/flags/reflection.h>

TURBO_DECLARE_FLAG(std::vector<std::string>, flags_file);
turbo::flat_hash_set<int> set{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

TURBO_FLAG(std::string, test_flag, "test", "test flag").on_validate(turbo::AllPassValidator<std::string>::validate);

TURBO_FLAG(int, gt_flag, 10, "test flag").on_validate(turbo::GeValidator<int, 5>::validate);

TURBO_FLAG(int, range_flag, 10, "test flag").on_validate(turbo::ClosedOpenInRangeValidator<int, 5,15>::validate);

TURBO_FLAG(int, inset_flag, 3, "test flag").on_validate(turbo::InSetValidator<int, set>::validate);

std::string_view prefix = "/opt/EA";
TURBO_FLAG(std::string, prefix_flag, "/opt/EA/inf", "test flag").on_validate(turbo::StartsWithValidator<prefix>::validate);

int main(int argc, char **argv) {
    turbo::setup_argv(argc, argv);
    turbo::load_flags({"conf.flags"});

    std::cout << "gt_flag: " << turbo::get_flag(FLAGS_gt_flag) << std::endl;

    turbo::set_flag(&FLAGS_gt_flag, 3);
    std::cout << "gt_flag: " << turbo::get_flag(FLAGS_gt_flag) << std::endl;

    auto *flag = turbo::find_command_line_flag("flags_file");

    flag->parse_from("con.flags,con1.flags", nullptr);
    for (auto &item : turbo::get_flag(FLAGS_flags_file)) {
        std::cout <<"flags_file: " << item << std::endl;

    }
    std::cout << "gt_flag: " <<FLAGS_gt_flag.name() << std::endl;

}