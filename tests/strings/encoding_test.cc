// Copyright (C) 2024 Kumo group inc.
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