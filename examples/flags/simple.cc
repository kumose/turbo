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
#include <turbo/version.h>
#include <iostream>
#include <string>

int main(int argc, char **argv) {

    turbo::cli::App app("K3Pi goofit fitter");
    // add version output
    app.set_version_flag("--version", TURBO_VERSION_STRING);
    std::string file;
    turbo::cli::Option *opt = app.add_option("-f,--file,file", file, "File name");

    int count{0};
    turbo::cli::Option *copt = app.add_option("-c,--count", count, "Counter");

    int v{0};
    turbo::cli::Option *flag = app.add_flag("--flag", v, "Some flag that can be passed multiple times");

    double value{0.0};  // = 3.14;
    app.add_option("-d,--double", value, "Some Value");

    TURBO_APP_PARSE(app, argc, argv);

    std::cout << "Working on file: " << file << ", direct count: " << app.count("--file")
              << ", opt count: " << opt->count() << '\n';
    std::cout << "Working on count: " << count << ", direct count: " << app.count("--count")
              << ", opt count: " << copt->count() << '\n';
    std::cout << "Received flag: " << v << " (" << flag->count() << ") times\n";
    std::cout << "Some value: " << value << '\n';

    return 0;
}
