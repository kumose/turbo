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
#include <turbo/random/fast_rand.h>
#include <turbo/times/time.h>
#include <algorithm>
#include <limits>

#include <turbo/log/logging.h>
#include <turbo/memory/scoped_ptr.h>
#include <gtest/gtest.h>

namespace {

const int kIntMin = std::numeric_limits<int>::min();
const int kIntMax = std::numeric_limits<int>::max();

}  // namespace

TEST(RandUtilTest, Sanity) {
    EXPECT_EQ(turbo::RandInt(0, 0), 0);
    EXPECT_EQ(turbo::RandInt(kIntMin, kIntMin), kIntMin);
    EXPECT_EQ(turbo::RandInt(kIntMax, kIntMax), kIntMax);

    for (int i = 0; i < 10; ++i) {
        uint64_t value = turbo::fast_rand_in(
            (uint64_t)0, std::numeric_limits<uint64_t>::max());
        if (value != std::numeric_limits<uint64_t>::min() &&
            value != std::numeric_limits<uint64_t>::max()) {
            break;
        } else {
            EXPECT_NE(9, i) << "Never meet random except min/max of uint64";
        }
    }
    for (int i = 0; i < 10; ++i) {
        int64_t value = turbo::fast_rand_in(
            std::numeric_limits<int64_t>::min(),
            std::numeric_limits<int64_t>::max());
        if (value != std::numeric_limits<int64_t>::min() &&
            value != std::numeric_limits<int64_t>::max()) {
            break;
        } else {
            EXPECT_NE(9, i) << "Never meet random except min/max of int64";
        }
    }
    EXPECT_EQ(turbo::fast_rand_in(-1, -1), -1);
    EXPECT_EQ(turbo::fast_rand_in(1, 1), 1);
    EXPECT_EQ(turbo::fast_rand_in(0, 0), 0);
    EXPECT_EQ(turbo::fast_rand_in(std::numeric_limits<int64_t>::min(),
                                  std::numeric_limits<int64_t>::min()),
              std::numeric_limits<int64_t>::min());
    EXPECT_EQ(turbo::fast_rand_in(std::numeric_limits<int64_t>::max(),
                                  std::numeric_limits<int64_t>::max()),
              std::numeric_limits<int64_t>::max());
    EXPECT_EQ(turbo::fast_rand_in(std::numeric_limits<uint64_t>::min(),
                                  std::numeric_limits<uint64_t>::min()),
              std::numeric_limits<uint64_t>::min());
    EXPECT_EQ(turbo::fast_rand_in(std::numeric_limits<uint64_t>::max(),
                                  std::numeric_limits<uint64_t>::max()),
              std::numeric_limits<uint64_t>::max());
}

TEST(RandUtilTest, RandDouble) {
    // Force 64-bit precision, making sure we're not in a 80-bit FPU register.
    volatile double number = turbo::RandDouble();
    EXPECT_GT(1.0, number);
    EXPECT_LE(0.0, number);

    volatile double number2 = turbo::fast_rand_double();
    EXPECT_GT(1.0, number2);
    EXPECT_LE(0.0, number2);
}

TEST(RandUtilTest, RandBytes) {
    const size_t buffer_size = 50;
    char buffer[buffer_size];
    memset(buffer, 0, buffer_size);
    turbo::RandBytes(buffer, buffer_size);
    std::sort(buffer, buffer + buffer_size);
    // Probability of occurrence of less than 25 unique bytes in 50 random bytes
    // is below 10^-25.
    EXPECT_GT(std::unique(buffer, buffer + buffer_size) - buffer, 25);
}

TEST(RandUtilTest, RandBytesAsString) {
    std::string random_string = turbo::RandBytesAsString(1);
    EXPECT_EQ(1U, random_string.size());
    random_string = turbo::RandBytesAsString(145);
    EXPECT_EQ(145U, random_string.size());
    char accumulator = 0;
    for (size_t i = 0; i < random_string.size(); ++i) {
        accumulator |= random_string[i];
    }
    // In theory this test can fail, but it won't before the universe dies of
    // heat death.
    EXPECT_NE(0, accumulator);
}

// Make sure that it is still appropriate to use RandGenerator in conjunction
// with std::random_shuffle().
TEST(RandUtilTest, RandGeneratorForRandomShuffle) {
    EXPECT_EQ(turbo::RandGenerator(1), 0U);
    EXPECT_LE(std::numeric_limits<ptrdiff_t>::max(),
              std::numeric_limits<int64_t>::max());
}

TEST(RandUtilTest, RandGeneratorIsUniform) {
    // Verify that RandGenerator has a uniform distribution. This is a
    // regression test that consistently failed when RandGenerator was
    // implemented this way:
    //
    //   return turbo::RandUint64() % max;
    //
    // A degenerate case for such an implementation is e.g. a top of
    // range that is 2/3rds of the way to MAX_UINT64, in which case the
    // bottom half of the range would be twice as likely to occur as the
    // top half. A bit of calculus care of jar@ shows that the largest
    // measurable delta is when the top of the range is 3/4ths of the
    // way, so that's what we use in the test.
    const uint64_t kTopOfRange = (std::numeric_limits<uint64_t>::max() / 4ULL) * 3ULL;
    const uint64_t kExpectedAverage = kTopOfRange / 2ULL;
    const uint64_t kAllowedVariance = kExpectedAverage / 50ULL;  // +/- 2%
    const int kMinAttempts = 1000;
    const int kMaxAttempts = 1000000;

    for (int round = 0; round < 2; ++round) {
        KLOG(INFO) << "Use " << (round == 0 ? "RandUtil" : "fast_rand");
        double cumulative_average = 0.0;
        int count = 0;
        while (count < kMaxAttempts) {
            uint64_t value = (round == 0 ? turbo::RandGenerator(kTopOfRange)
                              : turbo::fast_rand_less_than(kTopOfRange));
            cumulative_average = (count * cumulative_average + value) / (count + 1);

            // Don't quit too quickly for things to start converging, or we may have
            // a false positive.
            if (count > kMinAttempts &&
                double(kExpectedAverage - kAllowedVariance) < cumulative_average &&
                cumulative_average < double(kExpectedAverage + kAllowedVariance)) {
                break;
            }

            ++count;
        }

        ASSERT_LT(count, kMaxAttempts) << "Expected average was " <<
            kExpectedAverage << ", average ended at " << cumulative_average;
    }
}

TEST(RandUtilTest, RandUint64ProducesBothValuesOfAllBits) {
    // This tests to see that our underlying random generator is good
    // enough, for some value of good enough.
    const uint64_t kAllZeros = 0ULL;
    const uint64_t kAllOnes = ~kAllZeros;

    for (int round = 0; round < 2; ++round) {
        KLOG(INFO) << "Use " << (round == 0 ? "RandUtil" : "fast_rand");
        uint64_t found_ones = kAllZeros;
        uint64_t found_zeros = kAllOnes;
        bool fail = true;
        for (size_t i = 0; i < 1000; ++i) {
            uint64_t value = (round == 0 ? turbo::RandUint64() : turbo::fast_rand());
            found_ones |= value;
            found_zeros &= value;

            if (found_zeros == kAllZeros && found_ones == kAllOnes) {
                fail = false;
                break;
            }
        }
        if (fail) {
            FAIL() << "Didn't achieve all bit values in maximum number of tries.";
        }
    }
}

// Benchmark test for RandBytes().  Disabled since it's intentionally slow and
// does not test anything that isn't already tested by the existing RandBytes()
// tests.
TEST(RandUtilTest, DISABLED_RandBytesPerf) {
    // Benchmark the performance of |kTestIterations| of RandBytes() using a
    // buffer size of |kTestBufferSize|.
    const int kTestIterations = 10;
    const size_t kTestBufferSize = 1 * 1024 * 1024;

    turbo::scoped_ptr<uint8_t[]> buffer(new uint8_t[kTestBufferSize]);
    const auto now = turbo::Time::current_time();
    for (int i = 0; i < kTestIterations; ++i) {
        turbo::RandBytes(buffer.get(), kTestBufferSize);
    }
    const auto end = turbo::Time::current_time();

    KLOG(INFO) << "RandBytes(" << kTestBufferSize << ") took: "
              << turbo::Duration::to_microseconds(end - now) << "us";
}

TEST(RandUtilTest, fast_rand_perf) {
    const int kTestIterations = 1000000;
    const int kRange = 17;
    uint64_t s = 0;
    turbo::TimeCost tm;
    tm.reset();
    for (int i = 0; i < kTestIterations; ++i) {
        s += turbo::fast_rand_less_than(kRange);
    }
    tm.stop();
    KLOG(INFO) << "Each fast_rand_less_than took " << tm.n_elapsed() / kTestIterations
              << " ns, s=" << s
#if !defined(NDEBUG)
              << " (debugging version)";
#else
             ;
#endif
}
