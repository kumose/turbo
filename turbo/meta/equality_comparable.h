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
// Created by jeff on 24-6-1.
//

#pragma once


#include <memory>
#include <type_traits>
#include <utility>

#include <turbo/base/macros.h>

namespace turbo {
    /// CRTP helper for declaring equality comparison. Defines operator== and operator!=
    template<typename T>
    class EqualityComparable {
    public:
        ~EqualityComparable() {
            static_assert(
                    std::is_same<decltype(std::declval<const T>().equals(std::declval<const T>())),
                            bool>::value,
                    "EqualityComparable depends on the method T::equals(const T&) const");
        }

        template<typename... Extra>
        bool equals(const std::shared_ptr<T> &other, Extra &&... extra) const {
            if (other == nullptr) {
                return false;
            }
            return cast().equals(*other, std::forward<Extra>(extra)...);
        }

        struct PtrsEqual {
            bool operator()(const std::shared_ptr<T> &l, const std::shared_ptr<T> &r) const {
                return l->equals(*r);
            }
        };

        friend bool operator==(T const &a, T const &b) { return a.equals(b); }

        friend bool operator!=(T const &a, T const &b) { return !(a == b); }

    private:
        const T &cast() const { return static_cast<const T &>(*this); }
    };

}  // namespace turbo

