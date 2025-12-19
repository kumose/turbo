// Copyright (C) 2024 EA group inc.
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
