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
