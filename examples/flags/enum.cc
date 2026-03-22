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
#include <map>
#include <string>

enum class Level : int { High, Medium, Low };

int main(int argc, char **argv) {
    turbo::cli::App app;

    Level level{Level::Low};
    // specify string->value mappings
    std::map<std::string, Level> map{{"high", Level::High}, {"medium", Level::Medium}, {"low", Level::Low}};
    // CheckedTransformer translates and checks whether the results are either in one of the strings or in one of the
    // translations already
    app.add_option("-l,--level", level, "Level settings")
        ->required()
        ->transform(turbo::cli::CheckedTransformer(map, turbo::cli::ignore_case));

    TURBO_APP_PARSE(app, argc, argv);

    // CLI11's built in enum streaming can be used outside CLI11 like this:
    using turbo::cli::enums::operator<<;
    std::cout << "Enum received: " << level << '\n';

    return 0;
}
