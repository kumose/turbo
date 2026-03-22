//
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
//
// Created by jeff on 24-6-5.
//

#pragma once


#include <array>
#include <cstdint>
#include <turbo/strings/string_view.h>

namespace turbo {

    template <typename T> class ArrayRef;

    class SHA256 {
    public:
        explicit SHA256() { init(); }

        /// Reinitialize the internal state
        void init();

        /// Digest more data.
        void update(ArrayRef<uint8_t> Data);

        /// Digest more data.
        void update(std::string_view Str);

        /// Return the current raw 256-bits SHA256 for the digested
        /// data since the last call to init(). This call will add data to the
        /// internal state and as such is not suited for getting an intermediate
        /// result (see result()).
        std::array<uint8_t, 32> final();

        /// Return the current raw 256-bits SHA256 for the digested
        /// data since the last call to init(). This is suitable for getting the
        /// SHA256 at any time without invalidating the internal state so that more
        /// calls can be made into update.
        std::array<uint8_t, 32> result();

        /// Returns a raw 256-bit SHA256 hash for the given data.
        static std::array<uint8_t, 32> hash(ArrayRef<uint8_t> Data);

    private:
        /// Define some constants.
        /// "static constexpr" would be cleaner but MSVC does not support it yet.
        enum { BLOCK_LENGTH = 64 };
        enum { HASH_LENGTH = 32 };

        // Internal State
        struct {
            union {
                uint8_t C[BLOCK_LENGTH];
                uint32_t L[BLOCK_LENGTH / 4];
            } Buffer;
            uint32_t State[HASH_LENGTH / 4];
            uint32_t ByteCount;
            uint8_t BufferOffset;
        } InternalState;

        // Helper
        void writebyte(uint8_t data);
        void hashBlock();
        void addUncounted(uint8_t data);
        void pad();

        void final(std::array<uint32_t, HASH_LENGTH / 4> &HashResult);
    };

} // namespace turbo
