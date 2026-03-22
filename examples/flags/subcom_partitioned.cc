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
#include <CLI/Timer.hpp>
#include <iostream>
#include <memory>
#include <string>

int main(int argc, char **argv) {
    turbo::cli::AutoTimer give_me_a_name("This is a timer");

    turbo::cli::App app("K3Pi goofit fitter");

    turbo::cli::App_p impOpt = std::make_shared<turbo::cli::App>("Important");
    std::string file;
    turbo::cli::Option *opt = impOpt->add_option("-f,--file,file", file, "File name")->required();

    int count{0};
    turbo::cli::Option *copt = impOpt->add_flag("-c,--count", count, "Counter")->required();

    turbo::cli::App_p otherOpt = std::make_shared<turbo::cli::App>("Other");
    double value{0.0};  // = 3.14;
    otherOpt->add_option("-d,--double", value, "Some Value");

    // add the subapps to the main one
    app.add_subcommand(impOpt);
    app.add_subcommand(otherOpt);

    try {
        app.parse(argc, argv);
    } catch(const turbo::cli::ParseError &e) {
        return app.exit(e);
    }

    std::cout << "Working on file: " << file << ", direct count: " << impOpt->count("--file")
              << ", opt count: " << opt->count() << '\n';
    std::cout << "Working on count: " << count << ", direct count: " << impOpt->count("--count")
              << ", opt count: " << copt->count() << '\n';
    std::cout << "Some value: " << value << '\n';

    return 0;
}
