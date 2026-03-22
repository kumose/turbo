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

#include <turbo/container/flat_hash_set.h>

namespace turbo {

    // Cache policy abstract base class
    // Key Type of a key a policy works with
    template<typename Key>
    class CachePolicyBase {
    public:
        virtual ~CachePolicyBase() = default;

         // insert a key into the cache
         // key Key that should be used by the policy
        virtual void insert(const Key &key) = 0;

         // Handle request to the key-element in a cache
        virtual void touch(const Key &key) = 0;

         // erase a key from the cache
        virtual void erase(const Key &key) = 0;

         // return a key of a displacement candidate
         // return Replacement candidate according to selected policy
        virtual const Key &repl_candidate() const = 0;
    };

    // Basic no caching policy class
    // Preserve any key provided. erase procedure can get rid of any added keys
    // without specific rules: a replacement candidate will be the first element in the
    // underlying container. As unordered container can be used in the implementation
    // there are no warranties that the first/last added key will be erased
    // Key Type of a key a policy works with
    template<typename Key>
    class NoCachePolicy : public CachePolicyBase<Key> {
    public:
        NoCachePolicy() = default;

        ~NoCachePolicy() noexcept override = default;

        void insert(const Key &key) override {
            key_storage.emplace(key);
        }

        void touch(const Key &key) noexcept override {
            // do not do anything
            (void) key;
        }

        void erase(const Key &key) noexcept override {
            key_storage.erase(key);
        }

        // return a key of a displacement candidate
        const Key &repl_candidate() const noexcept override {
            return *key_storage.cbegin();
        }

    private:
        turbo::flat_hash_set<Key> key_storage;
    };
} // namespace turbo

