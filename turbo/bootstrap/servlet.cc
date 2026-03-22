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

#include <turbo/bootstrap/servlet.h>
#include <turbo/flags/declare.h>
#include <turbo/flags/parse.h>
#include <turbo/log/flags.h>
#include <turbo/log/logging.h>
#include <turbo/strings/str_join.h>
#include <vector>
#include <string>

/// log
TURBO_DECLARE_FLAG(std::vector<std::string>, flags_file);
TURBO_DECLARE_FLAG(int, stderr_threshold);
TURBO_DECLARE_FLAG(int, min_log_level);
TURBO_DECLARE_FLAG(std::string, backtrace_log_at);
TURBO_DECLARE_FLAG(bool, log_with_prefix);
TURBO_DECLARE_FLAG(int, verbosity);
TURBO_DECLARE_FLAG(std::string, vlog_module);
/// system
TURBO_DECLARE_FLAG(bool, reuse_port);
TURBO_DECLARE_FLAG(bool, reuse_addr);
TURBO_DECLARE_FLAG(bool, reuse_uds_path);
TURBO_DECLARE_FLAG(bool, run_command_through_clone);
TURBO_DECLARE_FLAG(std::vector<std::string>, from_env);
TURBO_DECLARE_FLAG(std::vector<std::string>, try_from_env);
TURBO_DECLARE_FLAG(std::vector<std::string>, undef_ok);

TURBO_FLAG(bool, no_log_init, false, "do not init log by servlet");

namespace turbo {
    Servlet Servlet::instance_;

    Servlet::Servlet() {
    }

    void Servlet::setup() {
        run_app_ = app_.add_subcommand("run", "servlet run command");
    }

    std::pair<bool,int>  Servlet::launch(int argc, char **argv) {
        try {
            setup_argv(argc, argv);
            app_.parse_complete_callback([argc, argv](){
                std::vector<char*> positional_args;
                std::vector<turbo::UnrecognizedFlag> unrecognized_flags;
                turbo::parse_turbo_flags_only(argc, argv, positional_args, unrecognized_flags);
                if(turbo::get_flag(FLAGS_no_log_init) || turbo::get_flag(FLAGS_log_type) == 0){
                    return ;
                }
                if(turbo::get_flag(FLAGS_log_base_filename).empty()) {
                    auto default_log_name = make_default_log_filename(argv[0]);
                    turbo::set_flag(&FLAGS_log_base_filename, default_log_name);
                    std::cerr<<"log name not set, set to : "<<default_log_name<<std::endl;
                }
                turbo::setup_log_by_flags();
            });
            app_.parse(argc, argv);
        } catch (const turbo::cli::ParseError &e) {
            return {true,app_.exit(e)};
        }
        launch_params_ = &get_argv();
        return {false,0};
    }

    Servlet &Servlet::set_version(const std::string &version) {
        app_.set_version_flag("--version", version);
        return *this;
    }

    Servlet &Servlet::set_description(const std::string &version) {
        app_.description(version);
        return *this;
    }

    Servlet &Servlet::set_name(const std::string &name) {
        app_.name(name);
        return *this;
    }

    void Servlet::setup_log_option(turbo::cli::App *app) {
        auto lopt = app->add_option_group("log_option", "log flags options");
        lopt->enable_flags_option(FLAGS_stderr_threshold);
        lopt->enable_flags_option(FLAGS_min_log_level);
        lopt->enable_flags_option(FLAGS_backtrace_log_at);
        lopt->enable_flags_option(FLAGS_log_with_prefix);
        lopt->enable_flags_option( FLAGS_verbosity);
        lopt->enable_flags_option(FLAGS_vlog_module);
        lopt->enable_flags_option(FLAGS_log_type);
        lopt->enable_flags_option(FLAGS_no_log_init);
        lopt->enable_flags_option(FLAGS_log_base_filename);

    }

    void Servlet::setup_system_option( turbo::cli::App *app) {
        auto lopt = app->add_option_group("system", "system options");
        lopt->enable_flags_option(FLAGS_reuse_port);
        lopt->enable_flags_option(FLAGS_reuse_addr);
        lopt->enable_flags_option(FLAGS_reuse_uds_path);
        lopt->enable_flags_option(FLAGS_run_command_through_clone);
        lopt->enable_flags_option(FLAGS_from_env);
        lopt->enable_flags_option(FLAGS_try_from_env);
        lopt->enable_flags_option(FLAGS_undef_ok);
    }

    void Servlet::setup_flags_file(turbo::cli::App *app) {
        app->enable_flags_option(FLAGS_flags_file);
    }

    const std::vector<std::string> *Servlet::launch_params() const {
        return launch_params_;
    }

    Servlet &Servlet::add_default_flags_file(const std::string &file) {
        if(file.empty()) {
            return *this;
        }
        auto f = turbo::get_flag(FLAGS_flags_file);
        f.push_back(file);
        turbo::set_flag(&FLAGS_flags_file, f);
        return *this;
    }

    Servlet &Servlet::add_default_flags_files(const std::vector<std::string> &files) {
        for(const auto& file : files) {
            add_default_flags_file(file);
        }
        return *this;
    }

    Servlet &Servlet::clear_default_flags_files() {
        turbo::set_flag(&FLAGS_flags_file, std::vector<std::string>{});
        return *this;
    }

    std::string Servlet::default_flags_file() const {
        auto f = turbo::get_flag(FLAGS_flags_file);
        if(f.empty()) {
            return "";
        }
        std::string  result;
        for(const auto& file : f) {
            result += file + " ";
        }
        return result;
    }

}  // namespace turbo
