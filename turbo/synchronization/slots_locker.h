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
// Created by jeff on 24-6-6.
//
#pragma once

#include <functional>
#include <mutex>
#include <shared_mutex>
#include <set>
#include <string>
#include <vector>
#include <turbo/meta/type_traits.h>
#include <turbo/meta/transparent.h>
#include <turbo/container/hash_container_defaults.h>

namespace turbo {

    template<typename M>
    struct is_shared_mutex : public std::false_type {
    };

    template<>
    struct is_shared_mutex<std::shared_mutex> : public std::true_type {
    };

    class SharedMutex : public Mutex {
    public:
        SharedMutex() = default;

        ~SharedMutex() = default;

        void lock() {
            WriterLock();
        }

        void unlock() {
            WriterUnlock();
        }

        void lock_shared() {
            WriterLock();
        }

        void unlock_shared() {
            WriterUnlock();
        }
    };

    class NullMutex {
    public:
        NullMutex() = default;

        ~NullMutex() = default;

        void lock() {

        }

        void unlock() {

        }

        void lock_shared() {

        }

        void unlock_shared() {

        }
    };

    template<>
    struct is_shared_mutex<SharedMutex> : public std::true_type {
    };

    template<>
    struct is_shared_mutex<NullMutex> : public std::true_type {
    };

    template<typename Key, typename MUTEX=std::mutex, typename Hash=DefaultHashContainerHash<Key>>
    class SlotsLock {
    public:
        using mutex_type = MUTEX;
        using key_type = Key;
        using hash_type = Hash;
        template<class K>
        using key_arg = typename transparent_type<is_transparent<Hash>::value>::template type<K, key_type>;
        static constexpr bool is_trans = is_transparent<Hash>::value;
    public:
        explicit SlotsLock(unsigned hash_power)
                : hash_power_(hash_power), hash_mask_((1U << hash_power) - 1), mutex_pool_(size()) {}

        unsigned size() const { return (1U << hash_power_); }

        ~SlotsLock() = default;

        SlotsLock(const SlotsLock &) = delete;

        SlotsLock &operator=(const SlotsLock &) = delete;


        void lock(const key_type &key) { mutex_pool_[hash(key)].lock(); }

        void unLock(const key_type &key) { mutex_pool_[hash(key)].unlock(); }

        template<typename K = key_type>
        mutex_type *get(const key_arg<K> &key) {
            return &mutex_pool_[hash(key)];
        }

        template<typename Keys, typename K = typename sequence_container<Keys>::value_type>
        std::vector<mutex_type *> multi_get(const Keys &keys) {
            static_assert(sequence_container<Keys>::value, "keys must be sequence container like std::vector, std::list, eg.");
            std::set<unsigned, std::greater<unsigned>> to_acquire_indexes;
            // We are using the `set` to avoid retrieving the mutex, as well as guarantee to retrieve
            // the order of locks.
            //
            // For example, we need lock the key `A` and `B` and they have the same lock hash
            // index, it will be deadlock if lock the same mutex twice. Besides, we also need
            // to order the mutex before acquiring locks since different threads may acquire
            // same keys with different order.
            for (const auto &key: keys) {
                to_acquire_indexes.insert(hash(key));
            }

            std::vector<mutex_type *> locks;
            locks.reserve(to_acquire_indexes.size());
            for (auto index: to_acquire_indexes) {
                locks.emplace_back(&mutex_pool_[index]);
            }
            return locks;
        }

    private:

        struct HashOperator {
            template<class K, class... Args>
            size_t operator()(const K &key, Args &&...) const {
                return h(key);
            }

            const hash_type &h;
        };

        template<typename K = key_type>
        unsigned hash(const key_arg<K> &key) const {
            return hash_factor_(key) & hash_mask_;
        }

        unsigned hash_power_;
        unsigned hash_mask_;
        std::vector<mutex_type> mutex_pool_;
        hash_type hash_factor_;

    };

    template<typename LM>
    class UniqueLockGuard {
    public:
        using key_type = typename LM::key_type;
        using mutex_type = typename LM::mutex_type;
        template<class K>
        using key_arg = typename transparent_type<LM::is_trans>::template type<K, key_type>;
    public:

        template<typename Keys,
                typename std::enable_if<sequence_container<Keys>::value, int>::type = 0
                >
        explicit UniqueLockGuard(LM *lock_mgr, const Keys &keys) : locks_(lock_mgr->multi_get(keys)) {
            for (const auto &iter: locks_) {
                iter->lock();
            }
        }

        //template<typename Keys, typename std::enable_if<std::is_same_v<Keys,Key_type>,int>::type =0>
        template<typename K = key_type, typename std::enable_if<!sequence_container<K>::value, int>::type = 0>
        explicit UniqueLockGuard(LM *lock_mgr, const key_arg<K> &key) : locks_({lock_mgr->get(key)}) {
            for (const auto &iter: locks_) {
                iter->lock();
            }
        }

        ~UniqueLockGuard() {
            // Lock with order `A B C` and unlock should be `C B A`
            for (auto iter = locks_.rbegin(); iter != locks_.rend(); ++iter) {
                (*iter)->unlock();
            }
        }

        UniqueLockGuard(const UniqueLockGuard &) = delete;

        UniqueLockGuard &operator=(const UniqueLockGuard &) = delete;

        UniqueLockGuard(UniqueLockGuard &&guard) : locks_(std::move(guard.locks_)) {}

    private:
        std::vector<mutex_type *> locks_;
    };

    template<typename LM>
    class SharedLockGuard {
    public:
        using key_type = typename LM::key_type;
        using mutex_type = typename LM::mutex_type;
        template<class K>
        using key_arg = typename transparent_type<LM::is_trans>::template type<K, key_type>;
        static_assert(is_shared_mutex<typename LM::mutex_type>::value, "must be a shared mutex type");
    public:

        template<typename Keys,
                typename std::enable_if<sequence_container<Keys>::value, int>::type = 0
        >
        explicit SharedLockGuard(LM *lock_mgr, const Keys &keys) : locks_(lock_mgr->multi_get(keys)) {
            for (const auto &iter: locks_) {
                iter->lock();
            }
        }
        template<typename K = key_type, typename std::enable_if<!sequence_container<K>::value, int>::type = 0>
        explicit SharedLockGuard(LM *lock_mgr, const key_arg<K> &key) : locks_({lock_mgr->get(key)}) {
            for (const auto &iter: locks_) {
                iter->lock();
            }
        }

        ~SharedLockGuard() {
            // Lock with order `A B C` and unlock should be `C B A`
            for (auto iter = locks_.rbegin(); iter != locks_.rend(); ++iter) {
                (*iter)->unlock();
            }
        }

        SharedLockGuard(const SharedLockGuard &) = delete;

        SharedLockGuard &operator=(const SharedLockGuard &) = delete;

        SharedLockGuard(SharedLockGuard &&guard) : locks_(std::move(guard.locks_)) {}

    private:
        std::vector<mutex_type *> locks_;
    };

}  // namespace turbo


