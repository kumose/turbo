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
