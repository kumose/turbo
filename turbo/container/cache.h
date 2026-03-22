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
#include <turbo/container/cache/cache_internal.h>
#include <turbo/container/cache/fifo_cache_policy.h>
#include <turbo/container/cache/lru_cache_policy.h>
#include <turbo/container/cache/lfu_cache_policy.h>

namespace turbo{

    template <typename Key, typename Value>
    using Cache = fixed_sized_cache<Key, Value, NoCachePolicy>;

    template <typename Key, typename Value>
    using LRUCache = fixed_sized_cache<Key, Value, LRUCachePolicy>;

    template <typename Key, typename Value>
    using LFUCache = fixed_sized_cache<Key, Value, LFUCachePolicy>;

    template <typename Key, typename Value>
    using FIFOCache = fixed_sized_cache<Key, Value, FIFOCachePolicy>;

}  // namespace turbo
