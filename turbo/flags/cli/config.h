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

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <turbo/flags/cli/app.h>
#include <turbo/flags/cli/config_fwd.h>
#include <turbo/flags/cli/string_tools.h>
namespace turbo::cli {

namespace detail {

std::string convert_arg_for_ini(const std::string &arg,
                                char stringQuote = '"',
                                char literalQuote = '\'',
                                bool disable_multi_line = false);

/// Comma separated join, adds quotes if needed
std::string ini_join(const std::vector<std::string> &args,
                     char sepChar = ',',
                     char arrayStart = '[',
                     char arrayEnd = ']',
                     char stringQuote = '"',
                     char literalQuote = '\'');

void clean_name_string(std::string &name, const std::string &keyChars);

std::vector<std::string> generate_parents(const std::string &section, std::string &name, char parentSeparator);

/// assuming non default segments do a check on the close and open of the segments in a configItem structure
void checkParentSegments(std::vector<ConfigItem> &output, const std::string &currentSection, char parentSeparator);
}  // namespace detail

}  // namespace turbo::cli