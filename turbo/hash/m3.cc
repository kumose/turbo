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
