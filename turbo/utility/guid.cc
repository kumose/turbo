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

namespace turbo {

    bool IsValidGUID(const std::string& guid) {
      const size_t kGUIDLength = 36U;
      if (guid.length() != kGUIDLength)
        return false;

      const std::string hexchars = "0123456789ABCDEF";
      for (uint32_t i = 0; i < guid.length(); ++i) {
        char current = guid[i];
        if (i == 8 || i == 13 || i == 18 || i == 23) {
          if (current != '-')
            return false;
        } else {
          if (hexchars.find(current) == std::string::npos)
            return false;
        }
      }

      return true;
    }

}  // namespace turbo
