// Copyright (C) 2024 EA group inc.
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

#include <turbo/strings/encoding.h>
#include <cfloat>
#include <climits>
#include <cstdint>
#include <cstring>
#include <string>
#include <utility>

namespace turbo {
    inline uint64_t EncodeDoubleToUInt64(double value) {
        uint64_t result = 0;

        __builtin_memcpy(&result, &value, sizeof(value));

        if ((result >> 63) == 1) {
            // signed bit would be zero
            result ^= 0xffffffffffffffff;
        } else {
            // signed bit would be one
            result |= 0x8000000000000000;
        }

        return result;
    }

    inline double DecodeDoubleFromUInt64(uint64_t value) {
        if ((value >> 63) == 0) {
            value ^= 0xffffffffffffffff;
        } else {
            value &= 0x7fffffffffffffff;
        }

        double result = 0;
        __builtin_memcpy(&result, &value, sizeof(result));

        return result;
    }

    char *encode_double(char *buf, double value) { return encode_fixed64(buf, EncodeDoubleToUInt64(value)); }

    void encode_double(std::string *dst, double value) { encode_fixed64(dst, EncodeDoubleToUInt64(value)); }

    double decode_double(const char *ptr) { return DecodeDoubleFromUInt64(decode_fixed64(ptr)); }

    bool decode_double(std::string_view *input, double *value) {
        if (input->size() < sizeof(double)) return false;
        *value = decode_double(input->data());
        input->remove_prefix(sizeof(double));
        return true;
    }

    char *encode_varint32(char *dst, uint32_t v) {
        // Operate on characters as unsigneds
        auto *ptr = reinterpret_cast<unsigned char *>(dst);
        do {
            *ptr = 0x80 | v;
            v >>= 7, ++ptr;
        } while (v != 0);
        *(ptr - 1) &= 0x7F;
        return reinterpret_cast<char *>(ptr);
    }

    void encode_varint32(std::string *dst, uint32_t v) {
        char buf[5];
        char *ptr = encode_varint32(buf, v);
        dst->append(buf, static_cast<size_t>(ptr - buf));
    }

    const char *get_varint32_ptr_fallback(const char *p, const char *limit, uint32_t *value) {
        uint32_t result = 0;
        for (uint32_t shift = 0; shift <= 28 && p < limit; shift += 7) {
            uint32_t byte = static_cast<unsigned char>(*p);
            p++;
            if (byte & 0x80) {
                // More bytes are present
                result |= ((byte & 0x7F) << shift);
            } else {
                result |= (byte << shift);
                *value = result;
                return p;
            }
        }
        return nullptr;
    }

    const char *get_varint32_ptr(const char *p, const char *limit, uint32_t *value) {
        if (p < limit) {
            uint32_t result = static_cast<unsigned char>(*p);
            if ((result & 0x80) == 0) {
                *value = result;
                return p + 1;
            }
        }
        return get_varint32_ptr_fallback(p, limit, value);
    }

    bool decode_varint32(std::string_view *input, uint32_t *value) {
        const char *p = input->data();
        const char *limit = p + input->size();
        const char *q = get_varint32_ptr(p, limit, value);
        if (q == nullptr) {
            return false;
        } else {
            *input = std::string_view(q, static_cast<size_t>(limit - q));
            return true;
        }
    }
}  // namespace turbo
