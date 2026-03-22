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



#include <turbo/random/generator.h>
#include <random>
#include <mutex>

namespace turbo {


    namespace {

        int64_t get_id_internal() {
#ifdef _WIN32
            return GetCurrentProcessId();
#else
            return getpid();
#endif
        }

        std::mt19937_64 get_seed_generator() {
            // Initialize Mersenne Twister PRNG with a true random seed.
            // Make sure to mix in process id to minimize risks of clashes when parallel testing.
#ifdef ARROW_VALGRIND
            // Valgrind can crash, hang or enter an infinite loop on std::random_device,
          // use a crude initializer instead.
          const uint8_t dummy = 0;
          ARROW_UNUSED(dummy);
          std::mt19937_64 seed_gen(reinterpret_cast<uintptr_t>(&dummy) ^
                                   static_cast<uintptr_t>(GetPid()));
#else
            std::random_device true_random;
            std::mt19937_64 seed_gen(static_cast<uint64_t>(true_random()) ^
                                     (static_cast<uint64_t>(true_random()) << 32) ^
                                     static_cast<uint64_t>(get_id_internal()));
#endif
            return seed_gen;
        }

    }  // namespace

    int64_t get_random_seed() {
        // The process-global seed generator to aims to avoid calling std::random_device
        // unless truly necessary (it can block on some systems, see ARROW-10287).
        static auto seed_gen = get_seed_generator();
        static std::mutex seed_gen_mutex;

        std::lock_guard<std::mutex> lock(seed_gen_mutex);
        return static_cast<int64_t>(seed_gen());
    }

    void rand_int(int64_t n,int min_value, int max_value, int *out) {
        rand_uniform_int(n, min_value, max_value, out);
    }

    void rand_int(int64_t n,int min_value, int max_value, std::vector<int> *out) {
        turbo::BitGen gen;
        out->clear();
        out->resize(n);
        turbo::uniform_int_distribution<int> d(min_value, max_value);
        std::generate(out->begin(), out->end(), [&d, &gen] { return static_cast<int>(d(gen)); });
    }

    void random_bytes(int64_t n, uint8_t *out) {
        InsecureBitGen gen;
        std::uniform_int_distribution<uint32_t> d(0, std::numeric_limits<uint8_t>::max());
        std::generate(out, out + n, [&d, &gen] { return static_cast<uint8_t>(d(gen)); });
    }

    std::string random_string(int64_t n) {
        std::string s;
        s.resize(static_cast<size_t>(n));
        random_bytes(n, reinterpret_cast<uint8_t*>(&s[0]));
        return s;
    }

    void random_ascii(int64_t n, uint8_t* out) {
        rand_uniform_int(n, static_cast<int32_t>('A'), static_cast<int32_t>('z'), out);
    }

    std::string random_ascii(int64_t n) {
        std::string s;
        s.resize(static_cast<size_t>(n));
        random_ascii(n, reinterpret_cast<uint8_t*>(&s[0]));
        return s;
    }

    void random_is_valid(int64_t n, double pct_null, std::vector<bool>* is_valid) {
        InsecureBitGen gen;
        uniform_real_distribution<double> d(0.0, 1.0);
        is_valid->resize(n, false);
        std::generate(is_valid->begin(), is_valid->end(),
                      [&d, &gen, &pct_null] { return d(gen) > pct_null; });
    }
}  // namespace turbo
