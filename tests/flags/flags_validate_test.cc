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
// Created by jeff on 24-6-5.
//

#include <turbo/flags/commandlineflag.h>
#include <turbo/flags/flag.h>
#include <turbo/flags/reflection.h>
#include <gtest/gtest.h>

TURBO_FLAG(int, test_validate_flag, 0, "test validate flag").on_validate([](std::string_view intvalue, std::string* error) noexcept ->bool {
    int val;
    auto r = turbo::parse_flag(intvalue, &val, error);
    if (!r) {
        return false;
    }
    if (val < 0 || val > 100) {
        *error = "value must be in [0, 100]";
        return false;
    }
    return true;
}).on_update([]() noexcept {
    std::cout << "test_validate_flag updated" << std::endl;
});

TEST(FlagsValidateTest, ValidateFlag) {
    std::string error;
    auto cl = turbo::find_command_line_flag("test_validate_flag");
    EXPECT_TRUE(cl != nullptr);
    EXPECT_TRUE(cl->has_user_validator());
    EXPECT_TRUE(cl->user_validate("0", &error));
    EXPECT_TRUE(cl->user_validate("100", &error));
    EXPECT_FALSE(cl->user_validate("-1", &error));
    EXPECT_FALSE(cl->user_validate("101", &error));
    cl->parse_from("50", &error);
}