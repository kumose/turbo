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

#pragma once

#include <string>
#include <vector>
#include <turbo/base/macros.h>

namespace turbo::cli {
namespace detail {
#ifdef _WIN32
/// Decode and return UTF-8 argv from GetCommandLineW.
std::vector<std::string> compute_win32_argv();
#endif
}  // namespace detail
}  // namespace turbo::cli

namespace turbo {

    void setup_argv(int argc, char** argv);

    const std::vector<std::string>& get_argv();

    void load_flags(const std::vector<std::string>& flags_files);
    void load_flags();

}  // namespace turbo