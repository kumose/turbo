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

#include <turbo/base/macros.h>

namespace turbo {

    // Generate a 128-bit random GUID of the form: "%08X-%04X-%04X-%04X-%012llX".
    // If GUID generation fails an empty string is returned.
    // The POSIX implementation uses psuedo random number generation to create
    // the GUID.  The Windows implementation uses system services.
    TURBO_EXPORT std::string GenerateGUID();

    // Returns true if the input string conforms to the GUID format.
    TURBO_EXPORT bool IsValidGUID(const std::string& guid);

    #if defined(OS_POSIX)
    // For unit testing purposes only.  Do not use outside of tests.
    TURBO_EXPORT std::string RandomDataToGUIDString(const uint64_t bytes[2]);
    #endif

}  // namespace turbo
