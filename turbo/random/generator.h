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

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <memory>
#include <vector>
#include <turbo/random/random.h>
#include <turbo/container/span.h>

namespace turbo {

    // using std::random
    int64_t get_random_seed();

    template<typename T, typename U>
    void random_real(int64_t n, T min_value, T max_value,
                     std::vector<U> *out) {
        turbo::BitGen gen;
        uniform_real_distribution<T> d(min_value, max_value);
        out->resize(n, static_cast<T>(0));
        std::generate(out->begin(), out->end(), [&d, &gen] { return static_cast<U>(d(gen)); });
    }

    template<typename T, typename U>
    void random_real(int64_t n, turbo::span<T> tpl,
                     std::vector<U> *out) {
        turbo::BitGen gen;
        uniform_real_distribution<T> d(0ul, tpl.size());
        out->resize(n, static_cast<T>(0));
        std::generate(out->begin(), out->end(), [&d, &gen, &tpl] { return tpl[static_cast<size_t>(d(gen))]; });
    }

    template<typename T, typename U>
    void rand_uniform_int(int64_t n, T min_value, T max_value, U *out) {
        assert(out || (n == 0));
        turbo::BitGen gen;
        turbo::uniform_int_distribution<T> d(min_value, max_value);
        std::generate(out, out + n, [&d, &gen] { return static_cast<U>(d(gen)); });
    }

    template<typename T, typename U>
    void rand_uniform_int(int64_t n,turbo::span<T> tpl, U *out) {
        assert(out || (n == 0));
        turbo::BitGen gen;
        turbo::uniform_int_distribution<T> d(0ul, tpl.size());
        std::generate(out, out + n, [&d, &gen, &tpl] { return tpl[static_cast<U>(d(gen))]; });
    }


    void rand_int(int64_t n,int min_value, int max_value, int *out);

    void rand_int(int64_t n,int min_value, int max_value, std::vector<int> *out);

    void random_bytes(int64_t n, uint8_t *out);

    std::string random_string(int64_t n);

    void random_ascii(int64_t n, uint8_t *out);

    std::string random_ascii(int64_t n);

    void random_is_valid(int64_t n, double pct_null, std::vector<bool> *is_valid);

}  // namespace turbo
