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

#include <turbo/utility/guid.h>
#include <turbo/strings/str_format.h>
#include <turbo/random/rand_util.h>

namespace turbo {

    std::string GenerateGUID() {
      uint64_t sixteen_bytes[2] = { turbo::RandUint64(), turbo::RandUint64() };
      return RandomDataToGUIDString(sixteen_bytes);
    }

    // TODO(cmasone): Once we're comfortable this works, migrate Windows code to
    // use this as well.
    std::string RandomDataToGUIDString(const uint64_t bytes[2]) {
      return turbo::str_format("%08X-%04X-%04X-%04X-%012llX",
                          static_cast<unsigned int>(bytes[0] >> 32),
                          static_cast<unsigned int>((bytes[0] >> 16) & 0x0000ffff),
                          static_cast<unsigned int>(bytes[0] & 0x0000ffff),
                          static_cast<unsigned int>(bytes[1] >> 48),
                          bytes[1] & 0x0000ffffffffffffULL);
    }

}  // namespace guid
