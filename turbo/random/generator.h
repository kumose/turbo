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
