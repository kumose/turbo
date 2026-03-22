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

#include <type_traits>

namespace turbo {

    template <class, class = void>
    struct is_transparent : std::false_type {};

    template <class T>
    struct is_transparent<T, std::void_t<typename T::is_transparent>>
            : std::true_type {};

    template <bool is_transparent>
    struct transparent_type {
        // Transparent. Forward `K`.
        template <typename K, typename key_type>
        using type = K;
    };

    template <>
    struct transparent_type<false> {
        // Not transparent. Always use `key_type`.
        template <typename K, typename key_type>
        using type = key_type;
    };

}  // namespace turbo
