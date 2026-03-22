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

#pragma once

#include <memory>
#include <string>

#include <turbo/base/macros.h>
#include <turbo/flags/commandlineflag.h>
#include <turbo/flags/internal/commandlineflag.h>
#include <turbo/strings/string_view.h>

namespace turbo::flags_internal {


    // This class serves as a trampoline to access private methods of
    // CommandLineFlag. This class is intended for use exclusively internally inside
    // of the Turbo Flags implementation.
    class PrivateHandleAccessor {
    public:
        // Access to CommandLineFlag::TypeId.
        static FlagFastTypeId TypeId(const CommandLineFlag &flag);

        // Access to CommandLineFlag::SaveState.
        static std::unique_ptr<FlagStateInterface> SaveState(CommandLineFlag &flag);

        // Access to CommandLineFlag::is_specified_on_commandLine.
        static bool is_specified_on_commandLine(const CommandLineFlag &flag);

        // Access to CommandLineFlag::validate_input_value.
        static bool validate_input_value(const CommandLineFlag &flag,
                                       std::string_view value);

        // Access to CommandLineFlag::check_default_value_parsing_roundtrip.
        static void check_default_value_parsing_roundtrip(const CommandLineFlag &flag);

        static bool parse_from(CommandLineFlag &flag, std::string_view value,
                              flags_internal::FlagSettingMode set_mode,
                              flags_internal::ValueSource source, std::string &error);
    };

}  // namespace turbo::flags_internal
