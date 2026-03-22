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
// Created by jeff on 24-6-30.
//

#pragma once

#include <string>
#include <vector>
#include <turbo/flags/app.h>
#include <turbo/flags/reflection.h>

namespace turbo {
    class Servlet {
    public:

        static Servlet &instance() {
            return instance_;
        }

        std::pair<bool, int> launch(int argc, char **argv);

        Servlet &set_version(const std::string &version);

        Servlet &set_description(const std::string &version);

        Servlet &set_name(const std::string &name);

        turbo::cli::App *add_command(const std::string &name, const std::string &description) {
            auto cmd =  app_.add_subcommand(name, description);
            return cmd;
        }

        turbo::cli::App *command(const std::string &name) {
            return  app_.get_subcommand(name);
        }

        turbo::cli::App &root() {
            return app_;
        }

        turbo::cli::App *run_app() {
            if (!run_app_) {
                setup();
            }
            return run_app_;
        }

        const std::vector<std::string> *launch_params() const;

        Servlet &add_default_flags_file(const std::string &file);

        Servlet &add_default_flags_files(const std::vector<std::string> &files);

        Servlet &clear_default_flags_files();

        static void setup_log_option( turbo::cli::App *);

        static void setup_system_option( turbo::cli::App *);

        static void setup_flags_file(turbo::cli::App *app);

        static void setup_default_option( turbo::cli::App *app) {
            setup_log_option(app);
            setup_system_option(app);
            setup_flags_file(app);
        }

    private:
        Servlet();

        void setup();

        std::string default_flags_file() const;

    private:
        static Servlet instance_;
        turbo::cli::App app_;
        turbo::cli::App *run_app_ = nullptr;
        const std::vector<std::string> *launch_params_ = nullptr;
        std::vector<std::string> default_flags_files_;
    };


}  // namespace turbo

#ifndef TURBO_SERVLET_PARSE
#define TURBO_SERVLET_PARSE(argc, argv) TURBO_APP_PARSE(turbo::Servlet::instance().root(), (argc), (argv))
#endif