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
#include <memory>

class MyFormatter : public turbo::cli::Formatter {
  public:
    MyFormatter() : Formatter() {}
    std::string make_option_opts(const turbo::cli::Option *) const override { return " OPTION"; }
};

int main(int argc, char **argv) {
    turbo::cli::App app;
    app.set_help_all_flag("--help-all", "Show all help");

    auto fmt = std::make_shared<MyFormatter>();
    fmt->column_width(15);
    app.formatter(fmt);

    app.add_flag("--flag", "This is a flag");

    auto *sub1 = app.add_subcommand("one", "Description One");
    sub1->add_flag("--oneflag", "Some flag");
    auto *sub2 = app.add_subcommand("two", "Description Two");
    sub2->add_flag("--twoflag", "Some other flag");

    TURBO_APP_PARSE(app, argc, argv);

    std::cout << "This app was meant to show off the formatter, run with -h" << '\n';

    return 0;
}
