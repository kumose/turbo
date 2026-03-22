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

#include <turbo/crypto/sha256.h>
#include <turbo/container/array_ref.h>
#include "gtest/gtest.h"

using namespace turbo;

namespace {

    static std::string toHex(ArrayRef<uint8_t> Input) {
        static const char *const LUT = "0123456789abcdef";
        size_t Length = Input.size();

        std::string Output;
        Output.reserve(2 * Length);
        for (size_t i = 0; i < Length; ++i) {
            const unsigned char c = Input[i];
            Output.push_back(LUT[c >> 4]);
            Output.push_back(LUT[c & 15]);
        }
        return Output;
    }

    /// Tests an arbitrary set of bytes passed as \p Input.
    void TestSHA256Sum(ArrayRef<uint8_t> Input, std::string_view Final) {
        SHA256 Hash;
        Hash.update(Input);
        auto hash = Hash.final();
        auto hashStr = toHex(hash);
        EXPECT_EQ(hashStr, Final);
    }

    using KV = std::pair<const char *, const char *>;

    TEST(SHA256Test, SHA256) {
        std::array<KV, 5> testvectors{
                KV{"",
                   "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"},
                KV{"a",
                   "ca978112ca1bbdcafac231b39a23dc4da786eff8147c4e72b9807785afee48bb"},
                KV{"abc",
                   "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"},
                KV{"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
                   "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1"},
                KV{"abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklm"
                   "nopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
                   "cf5b16a778af8380036ce59e7b0492370b249b11e8f07a51afac45037afee9d1"}};

        for (auto input_expected : testvectors) {
            auto str = std::get<0>(input_expected);
            auto expected = std::get<1>(input_expected);
            TestSHA256Sum({reinterpret_cast<const uint8_t *>(str), strlen(str)},
                          expected);
        }

        std::string rep(1000, 'a');
        SHA256 Hash;
        for (int i = 0; i < 1000; ++i) {
            Hash.update({reinterpret_cast<const uint8_t *>(rep.data()), rep.size()});
        }
        auto hash = Hash.final();
        auto hashStr = toHex(hash);
        EXPECT_EQ(hashStr,
                  "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0");
    }

} // namespace
