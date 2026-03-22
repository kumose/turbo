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

#include <turbo/flags/app.h>
#include <iostream>
#include <string>

int main(int argc, const char *argv[]) {

    int value{0};
    turbo::cli::App app{"Test App"};
    app.add_option("-v", value, "value");

    auto *subcom = app.add_subcommand("sub", "")->prefix_command();
    TURBO_APP_PARSE(app, argc, argv);

    std::cout << "value =" << value << '\n';
    std::cout << "after Args:";
    for(const auto &aarg : subcom->remaining()) {
        std::cout << aarg << " ";
    }
    std::cout << '\n';
}
