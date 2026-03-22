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



#include <turbo/strings/encoding.h>

#include <gtest/gtest.h>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

TEST(Util, EncodeAndDecodeDouble) {
    std::vector<double> values = {-1234, -100.1234, -1.2345, 0, 1.2345, 100.1234, 1234};
    std::string prev_bytes;
    for (auto value: values) {
        std::string bytes;
        turbo::encode_double(&bytes, value);
        double got = turbo::decode_double(bytes.data());
        if (!prev_bytes.empty()) {
            ASSERT_LT(prev_bytes, bytes);
        }
        prev_bytes.assign(bytes);
        ASSERT_EQ(value, got);
    }
}

TEST(Util, EncodeAndDecodeInt32AsVarint32) {
    std::vector<uint32_t> values = {200, 65000, 16700000, 4294000000};
    std::vector<size_t> encoded_sizes = {2, 3, 4, 5};
    for (size_t i = 0; i < values.size(); ++i) {
        std::string buf;
        turbo::encode_varint32(&buf, values[i]);
        EXPECT_EQ(buf.size(), encoded_sizes[i]);
        uint32_t result = 0;
        std::string_view s(buf);
        turbo::decode_varint32(&s, &result);
        ASSERT_EQ(result, values[i]);
    }
}

TEST(Util, EncodeAndDecodeInt32AsVarint32_1) {
    std::vector<uint32_t> values = {200, 65000, 16700000, 4294000000};
    std::vector<size_t> encoded_sizes = {2, 3, 4, 5};
    std::string buf;
    size_t encoded = 0;
    for (size_t i = 0; i < values.size(); ++i) {
        turbo::encode_varint32(&buf, values[i]);
        encoded += encoded_sizes[i];
        EXPECT_EQ(buf.size(), encoded);
    }
    uint32_t result = 0;
    std::string_view s(buf);
    for (size_t i = 0; i < values.size(); ++i) {
        turbo::decode_varint32(&s, &result);
        encoded -= encoded_sizes[i];
        ASSERT_EQ(result, values[i]);
        ASSERT_EQ(s.size(), encoded);
    }
}

TEST(Util, EncodeAndDecodeFixedInt64) {
    std::vector<uint64_t> values = {200, 65000, 16700000, 4294000000};
    std::vector<size_t> encoded_sizes = {8, 8, 8, 8};
    std::string buf;
    size_t encoded = 0;
    for (size_t i = 0; i < values.size(); ++i) {
        turbo::encode_fixed64(&buf, values[i]);
        encoded += encoded_sizes[i];
        EXPECT_EQ(buf.size(), encoded);
    }
    uint64_t result = 0;
    std::string_view s(buf);
    for (size_t i = 0; i < values.size(); ++i) {
        turbo::decode_fixed64(&s, &result);
        encoded -= encoded_sizes[i];
        ASSERT_EQ(result, values[i]);
        ASSERT_EQ(s.size(), encoded);
    }
}