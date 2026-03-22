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
#include <turbo/random/rand_util.h>
#include <cmath>
#include <algorithm>
#include <limits>
#include <turbo/log/logging.h>

namespace turbo {

    int RandInt(int min, int max) {
        DKCHECK_LE(min, max);

        uint64_t range = static_cast<uint64_t>(max) - min + 1;
        int result = min + static_cast<int>(turbo::RandGenerator(range));
        DKCHECK_GE(result, min);
        DKCHECK_LE(result, max);
        return result;
    }

    double RandDouble() {
        return BitsToOpenEndedUnitInterval(turbo::RandUint64());
    }

    double BitsToOpenEndedUnitInterval(uint64_t bits) {
        // We try to get maximum precision by masking out as many bits as will fit
        // in the target type's mantissa, and raising it to an appropriate power to
        // produce output in the range [0, 1).  For IEEE 754 doubles, the mantissa
        // is expected to accommodate 53 bits.

        static_assert(std::numeric_limits<double>::radix == 2, "otherwise_use_scalbn");
        static const int kBits = std::numeric_limits<double>::digits;
        uint64_t random_bits = bits & ((uint64_t(1) << kBits) - 1);
        double result = ldexp(static_cast<double>(random_bits), -1 * kBits);
        DKCHECK_GE(result, 0.0);
        DKCHECK_LT(result, 1.0);
        return result;
    }

    uint64_t RandGenerator(uint64_t range) {
        DKCHECK_GT(range, 0u);
        // We must discard random results above this number, as they would
        // make the random generator non-uniform (consider e.g. if
        // MAX_UINT64 was 7 and |range| was 5, then a result of 1 would be twice
        // as likely as a result of 3 or 4).
        uint64_t max_acceptable_value =
                (std::numeric_limits<uint64_t>::max() / range) * range - 1;

        uint64_t value;
        do {
            value = turbo::RandUint64();
        } while (value > max_acceptable_value);

        return value % range;
    }

    template<class string_type>
    inline typename string_type::value_type *WriteInto(string_type *str,
                                                       size_t length_with_null) {
        DKCHECK_GT(length_with_null, 1u);
        str->reserve(length_with_null);
        str->resize(length_with_null - 1);
        return &((*str)[0]);
    }

    std::string RandBytesAsString(size_t length) {
        DKCHECK_GT(length, 0u);
        std::string result;
        RandBytes(WriteInto(&result, length + 1), length);
        return result;
    }

}  // namespace turbo
