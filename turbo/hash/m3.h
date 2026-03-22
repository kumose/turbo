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

#include <turbo/hash/internal/murmurhash3.h>
#include <turbo/container/array_ref.h>
#include <turbo/container/span.h>
#include <turbo/numeric/int128.h>
#include <string>
#include <vector>
#include <array>

namespace turbo {

    // simple hash
    uint32_t super_fast_hash(const char *data, int len);

    uint64_t murmur_hash_64(const uint8_t *data, int len, uint32_t seed = 0);

    void murmur_hash_128(const uint8_t *data, int len, uint32_t *out, uint32_t seed = 0);

    void murmur_hash_128(const uint8_t *data, int len, uint64_t *out, uint32_t seed = 0);


    class Mur32 {
    public:
        Mur32(uint32_t seed = 0);

        ~Mur32() = default;

        void update(ArrayRef<uint8_t> Data);

        void update(std::string_view Str);

        uint32_t final() const;

        static uint32_t hash(const uint8_t *data, int len, uint32_t seed = 0);

        static uint32_t hash(const std::string_view &sv, uint32_t seed = 0) {
            return hash(reinterpret_cast<const uint8_t *>(sv.data()), sv.size(), seed);
        }

        static uint32_t hash(const std::vector<std::string_view> &sv, uint32_t seed = 0);

        static uint32_t hash(const std::vector<std::string> &sv, uint32_t seed = 0);

    private:
        MurmurHash3_x86_32_Context _state;
    };

    class Mur64 {
    public:
        Mur64(uint32_t seed = 0);

        ~Mur64() = default;

        void update(ArrayRef<uint8_t> Data);

        void update(std::string_view Str);

        uint128 final() const;

        uint64_t final64() const;

    private:
        MurmurHash3_x64_128_Context _state;
    };
}  // namespace turbo

