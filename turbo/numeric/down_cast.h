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
// Created by jeff on 24-6-7.
//

#pragma once

#include <assert.h>
#include <type_traits>
#include <limits>
#include <turbo/base/casts.h>

namespace turbo {

    template<typename To, typename From>
    inline To down_cast(From *f) {
        static_assert(
                (std::is_base_of<From, typename std::remove_pointer<To>::type>::value),
                "target type not derived from source type");

#if !defined(__GNUC__) || defined(__GXX_RTTI)

        assert(f == nullptr || dynamic_cast<To>(f) != nullptr);
#endif

        return static_cast<To>(f);
    }

    template<typename To, typename From>
    inline To down_cast(From &f) {
        static_assert(std::is_lvalue_reference<To>::value,
                      "target type not a reference");
        static_assert(
                (std::is_base_of<From, typename std::remove_reference<To>::type>::value),
                "target type not derived from source type");

#if !defined(__GNUC__) || defined(__GXX_RTTI)

        assert(dynamic_cast<typename std::remove_reference<To>::type *>(&f) !=
               nullptr);
#endif

        return static_cast<To>(f);
    }

    inline float double_to_float(double value) {
        if (value < std::numeric_limits<float>::lowest())
            return -std::numeric_limits<float>::infinity();
        if (value > std::numeric_limits<float>::max())
            return std::numeric_limits<float>::infinity();

        return static_cast<float>(value);
    }

    inline float double_to_finite_float(double value) {
        if (value < std::numeric_limits<float>::lowest())
            return std::numeric_limits<float>::lowest();
        if (value > std::numeric_limits<float>::max())
            return std::numeric_limits<float>::max();

        return static_cast<float>(value);
    }


}  // namespace turbo

