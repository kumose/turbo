// Copyright (C) 2024 Kumo inc.
// Author: Jeff.li lijippy@163.com
// All rights reserved.
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
