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
// Created by jeff on 24-6-8.
//

#pragma once

#include <turbo/container/cache/cache_policy.h>
#include <turbo/container/flat_hash_map.h>
#include <list>

namespace turbo {

    /**
     * \brief FIFO (First in, first out) cache policy
     * \details FIFO policy in the case of replacement removes the first added element.
     *
     * That is, consider the following key adding sequence:
     * ```
     * A -> B -> C -> ...
     * ```
     * In the case a cache reaches its capacity, the FIFO replacement candidate policy
     * returns firstly added element `A`. To show that:
     * ```
     * # New key: X
     * Initial state: A -> B -> C -> ...
     * Replacement candidate: A
     * Final state: B -> C -> ... -> X -> ...
     * ```
     * An so on, the next candidate will be `B`, then `C`, etc.
     * \tparam Key Type of a key a policy works with
     */
    template<typename Key>
    class FIFOCachePolicy : public CachePolicyBase<Key> {
    public:
        using fifo_iterator = typename std::list<Key>::const_iterator;

        FIFOCachePolicy() = default;

        ~FIFOCachePolicy() = default;

        void insert(const Key &key) override {
            fifo_queue.emplace_front(key);
            key_lookup[key] = fifo_queue.begin();
        }

        // handle request to the key-element in a cache
        void touch(const Key &key) noexcept override {
            // nothing to do here in the FIFO strategy
            (void) key;
        }

        // handle element deletion from a cache
        void erase(const Key &key) noexcept override {
            auto element = key_lookup[key];
            fifo_queue.erase(element);
            key_lookup.erase(key);
        }

        // return a key of a replacement candidate
        const Key &repl_candidate() const noexcept override {
            return fifo_queue.back();
        }

    private:
        std::list<Key> fifo_queue;
        turbo::flat_hash_map<Key, fifo_iterator> key_lookup;
    };
} // namespace turbo

