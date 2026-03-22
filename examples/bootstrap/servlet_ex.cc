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

#include <turbo/bootstrap/servlet.h>
#include <turbo/version.h>
#include <iostream>
#include <string>
#include <turbo/flags/declare.h>
#include <turbo/flags/parse.h>
#include <turbo/log/logging.h>

TURBO_DECLARE_FLAG(std::vector<std::string>, flags_file);

TURBO_FLAG(int, gt_flag, 10, "test flag").on_validate(turbo::GeValidator<int, 5>::validate);

int main(int argc, char **argv) {
    auto &svt = turbo::Servlet::instance();
    svt.set_name("K3Pi")
        .set_version(TURBO_VERSION_STRING)
        .set_description("K3Pi goofit fitter");

    std::string file;
    turbo::cli::Option *opt = svt.run_app()->add_option("-f,--file,file", file, "File name");

    int count{0};
    turbo::cli::Option *copt = svt.run_app()->add_option("-c,--count", count, "Counter");

    svt.run_app()->add_option_function<int>("--gtf", [](const int&v) {
        std::cout << "gtf" ;
        }, "test flag");
    svt.root().add_option_function<int>("--gtff", [](const int&v) {
        std::cout << "root gtf" ;
    }, "test flag");



    int v{0};
    turbo::cli::Option *flag = svt.run_app()->add_flag("--flag", v, "Some flag that can be passed multiple times");

    double value{0.0};  // = 3.14;
    svt.run_app()->add_option("-d,--double", value, "Some Value");
    auto [exit, code] = svt.launch(argc, argv);
    if(exit) {
        return code;
    }

    VKLOG(20) << "Working on file: " << file << ", direct count: " << svt.run_app()->count("--file")
              << ", opt count: " << opt->count() << '\n';
    VKLOG(0) << "Working on file: " << file << ", direct count: " << svt.run_app()->count("--file")
             << ", opt count: " << opt->count() << '\n';
    KLOG(INFO) << "Working on count: " << count << ", direct count: " << svt.run_app()->count("--count")
              << ", opt count: " << copt->count() << '\n';
    KLOG(INFO)<< "Received flag: " << v << " (" << flag->count() << ") times\n";
    KLOG(INFO)<< "Some value: " << value << '\n';
    KLOG(INFO) << "gt_flag: " << turbo::get_flag(FLAGS_gt_flag) ;
    for(auto &item : turbo::get_flag(FLAGS_flags_file)) {
        KLOG(INFO) << "flags_file: " << item ;
    }

    return 0;
}
