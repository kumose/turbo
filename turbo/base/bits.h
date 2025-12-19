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

#include <turbo/log/logging.h>

namespace turbo {
    namespace bits {

        // Returns the integer i such as 2^i <= n < 2^(i+1)
        inline int Log2Floor(uint32_t n) {
            if (n == 0)
                return -1;
            int log = 0;
            uint32_t value = n;
            for (int i = 4; i >= 0; --i) {
                int shift = (1 << i);
                uint32_t x = value >> shift;
                if (x != 0) {
                    value = x;
                    log += shift;
                }
            }
            DKCHECK_EQ(value, 1u);
            return log;
        }

        // Returns the integer i such as 2^(i-1) < n <= 2^i
        inline int Log2Ceiling(uint32_t n) {
            if (n == 0) {
                return -1;
            } else {
                // Log2Floor returns -1 for 0, so the following works correctly for n=1.
                return 1 + Log2Floor(n - 1);
            }
        }

    }  // namespace bits
}  // namespace turbo
