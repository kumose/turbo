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
// -----------------------------------------------------------------------------
// File: algorithm.h
// -----------------------------------------------------------------------------
//
// This header file contains Google extensions to the standard <algorithm> C++
// header.

#ifndef TURBO_ALGORITHM_ALGORITHM_H_
#define TURBO_ALGORITHM_ALGORITHM_H_

#include <algorithm>
#include <iterator>
#include <type_traits>
#include <functional>
#include <utility>
#include <turbo/base/macros.h>
#include <numeric>

namespace turbo {

    // equal()
    // rotate()
    //
    // Historical note: Turbo once provided implementations of these algorithms
    // prior to their adoption in C++14. New code should prefer to use the std
    // variants.
    //
    // See the documentation for the STL <algorithm> header for more information:
    // https://en.cppreference.com/w/cpp/header/algorithm
    using std::equal;
    using std::rotate;

    // linear_search()
    //
    // Performs a linear search for `value` using the iterator `first` up to
    // but not including `last`, returning true if [`first`, `last`) contains an
    // element equal to `value`.
    //
    // A linear search is of O(n) complexity which is guaranteed to make at most
    // n = (`last` - `first`) comparisons. A linear search over short containers
    // may be faster than a binary search, even when the container is sorted.
    template<typename InputIterator, typename EqualityComparable>
    bool linear_search(InputIterator first, InputIterator last,
                       const EqualityComparable &value) {
        return std::find(first, last, value) != last;
    }


    template<typename T, typename Cmp = std::less<T>>
    std::vector<int64_t> arg_sort(const std::vector<T> &values, Cmp &&cmp = {}) {
        std::vector<int64_t> indices(values.size());
        std::iota(indices.begin(), indices.end(), 0);
        std::sort(indices.begin(), indices.end(),
                  [&](int64_t i, int64_t j) -> bool { return cmp(values[i], values[j]); });
        return indices;
    }

    template<typename T>
    size_t permute(const std::vector<int64_t> &indices, std::vector<T> *values) {
        if (indices.size() <= 1) {
            return indices.size();
        }

        // mask indicating which of values are in the correct location
        std::vector<bool> sorted(indices.size(), false);

        size_t cycle_count = 0;

        for (auto cycle_start = sorted.begin(); cycle_start != sorted.end();
             cycle_start = std::find(cycle_start, sorted.end(), false)) {
            ++cycle_count;

            // position in which an element belongs WRT sort
            auto sort_into = static_cast<int64_t>(cycle_start - sorted.begin());

            if (indices[sort_into] == sort_into) {
                // trivial cycle
                sorted[sort_into] = true;
                continue;
            }

            // resolve this cycle
            const auto end = sort_into;
            for (int64_t take_from = indices[sort_into]; take_from != end;
                 take_from = indices[sort_into]) {
                std::swap(values->at(sort_into), values->at(take_from));
                sorted[sort_into] = true;
                sort_into = take_from;
            }
            sorted[sort_into] = true;
        }

        return cycle_count;
    }
}  // namespace turbo

#endif  // TURBO_ALGORITHM_ALGORITHM_H_
