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

#include <turbo/hash/m3.h>
#include <turbo/hash/internal/murmurhash3.h>

namespace turbo {

    uint64_t murmur_hash_64(const uint8_t *data, int len, uint32_t seed) {
        uint64_t out[2];
        MurmurHash3_x64_128(data, len, seed, out);
        return out[0] * 101 + out[1];
    }

    void murmur_hash_128(const uint8_t *data, int len, uint32_t *out, uint32_t seed) {
        MurmurHash3_x86_128(data, len, seed, out);
    }

    void murmur_hash_128(const uint8_t *data, int len, uint64_t *out, uint32_t seed) {
        MurmurHash3_x64_128(data, len, seed, out);
    }

    Mur32::Mur32(uint32_t seed) {
        MurmurHash3_x86_32_Init(&_state, seed);
    }

    void Mur32::update(ArrayRef<uint8_t> data) {
        MurmurHash3_x86_32_Update(&_state,data.data(), data.size());
    }

    void Mur32::update(std::string_view str) {
        MurmurHash3_x86_32_Update(&_state,str.data(), str.size());
    }

    uint32_t Mur32::final() const {
        uint32_t ret;
        MurmurHash3_x86_32_Final(&ret, &_state);
        return ret;
    }

    uint32_t Mur32::hash(const uint8_t *data, int len, uint32_t seed) {
        uint32_t ret;
        MurmurHash3_x86_32(data, len, seed, &ret);
        return ret;
    }

    uint32_t Mur32::hash(const std::vector<std::string_view> &sv, uint32_t seed) {
        auto ret = seed;
        for(size_t i = 0 ; i < sv.size(); i++) {
            ret = hash(sv[i], ret);
        }
        return ret;
    }

    uint32_t Mur32::hash(const std::vector<std::string> &sv, uint32_t seed) {
        auto ret = seed;
        for(size_t i = 0 ; i < sv.size(); i++) {
            ret = hash(sv[i], ret);
        }
        return ret;
    }

    Mur64::Mur64(uint32_t seed) {
        MurmurHash3_x64_128_Init(&_state, seed);
    }

    void Mur64::update(ArrayRef<uint8_t> data) {
        MurmurHash3_x64_128_Update(&_state, data.data(),data.size());
    }

    void Mur64::update(std::string_view str) {
        MurmurHash3_x64_128_Update(&_state, str.data(),str.size());
    }

    uint128 Mur64::final() const {
        uint64_t out[2];
        MurmurHash3_x64_128_Final(out, &_state);
        return make_uint128(out[0], out[1]);
    }

    uint64_t Mur64::final64() const {
        uint64_t out[2];
        MurmurHash3_x64_128_Final(out, &_state);
        return out[0]  * 101 + out[1];
    }

}  // namespace turbo
