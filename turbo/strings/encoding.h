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

#pragma once

#include <turbo/base/endian.h>
#include <string>
#include <string_view>
#include <cstdint>
#include <string>

namespace turbo {

    template<typename T>
    inline char *encode_fixed(char *buf, T value) {
        if constexpr (is_little_endian()) {
            value = gbswap(value);
        }
        __builtin_memcpy(buf, &value, sizeof(value));
        return buf + sizeof(value);
    }

    template<typename T>
    inline void encode_fixed(std::string *buf, T value) {
        if constexpr (is_little_endian()) {
            value = gbswap(value);
        }
        buf->append(reinterpret_cast<const char*>(&value), sizeof(value));
    }


    inline char *encode_fixed8(char *buf, uint8_t value) { return encode_fixed(buf, value); }

    inline void encode_fixed8(std::string *buf, uint8_t value) {
        encode_fixed(buf, value);
    }

    inline char *encode_fixed16(char *buf, uint16_t value) { return encode_fixed(buf, value); }

    inline void encode_fixed16(std::string *buf, uint16_t value) { encode_fixed(buf, value); }

    inline char *encode_fixed32(char *buf, uint32_t value) { return encode_fixed(buf, value); }

    inline void encode_fixed32(std::string *buf, uint32_t value) { encode_fixed(buf, value); }

    inline char *encode_fixed64(char *buf, uint64_t value) { return encode_fixed(buf, value); }

    inline void encode_fixed64(std::string *buf, uint64_t value) { encode_fixed(buf, value); }

    inline char *encode_buffer(char *buf, std::string_view value) {
        __builtin_memcpy(buf, value.data(), value.size());

        return buf + value.size();
    }

    inline void encode_buffer(std::string *buf, std::string_view value) {
        buf->append(value);
    }

    template<typename T>
    constexpr T decode_fixed(const char *ptr) {
        T value = 0;

        __builtin_memcpy(&value, ptr, sizeof(value));

        return is_little_endian() ? gbswap(value) : value;
    }

    inline uint8_t decode_fixed8(const char *ptr) { return decode_fixed<uint8_t>(ptr); }

    inline uint16_t decode_fixed16(const char *ptr) { return decode_fixed<uint16_t>(ptr); }

    inline uint32_t decode_fixed32(const char *ptr) { return decode_fixed<uint32_t>(ptr); }

    inline uint64_t decode_fixed64(const char *ptr) { return decode_fixed<uint64_t>(ptr); }

    template<typename T>
    bool decode_fixed(std::string_view *input, T *value) {
        if (input->size() < sizeof(T)) return false;
        *value = decode_fixed<T>(input->data());
        *input = input->substr(sizeof(T));
        return true;
    }

    inline bool decode_fixed8(std::string_view *input, uint8_t *value) { return decode_fixed(input, value); }

    inline bool decode_fixed16(std::string_view *input, uint16_t *value) { return decode_fixed(input, value); }

    inline bool decode_fixed32(std::string_view *input, uint32_t *value) { return decode_fixed(input, value); }

    inline bool decode_fixed64(std::string_view *input, uint64_t *value) { return decode_fixed(input, value); }

    inline void encode_sized_string(std::string *dst,std::string_view value) {
        encode_fixed32(dst, value.size());
        dst->append(value);
    }

    inline bool decode_sized_string(std::string_view *input, std::string_view *value) {
        uint32_t size = 0;
        if (!decode_fixed32(input, &size)) return false;

        if (input->size() < size) return false;
        *value = std::string_view(input->data(), size);
        input->remove_prefix(size);
        return true;
    }

    char *encode_double(char *buf, double value);

    void encode_double(std::string *dst, double value);

    double decode_double(const char *ptr);

    bool decode_double(std::string_view *input, double *value);

    char *encode_varint32(char *dst, uint32_t v);

    void encode_varint32(std::string *dst, uint32_t v);

    bool decode_varint32(std::string_view *input, uint32_t *value);

}  // namespace turbo
