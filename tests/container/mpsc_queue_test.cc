// Copyright (C) 2024 Kumo inc.
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


#include <gtest/gtest.h>
#include <pthread.h>
#include <turbo/container/mpsc_queue.h>
#include <mutex>

namespace {

    const uint MAX_COUNT = 1000000;

    void Consume(turbo::MPSCQueue<uint> &q, bool allow_empty) {
        uint i = 0;
        uint empty_count = 0;
        while (true) {
            uint d;
            if (!q.Dequeue(d)) {
                ASSERT_TRUE(allow_empty);
                ASSERT_LT(empty_count++, (const uint) 10000);
                ::usleep(10 * 1000);
                continue;
            }
            ASSERT_EQ(i++, d);
            if (i == MAX_COUNT) {
                break;
            }
        }
    }

    void *ProduceThread(void *arg) {
        auto q = (turbo::MPSCQueue<uint> *) arg;
        for (uint i = 0; i < MAX_COUNT; ++i) {
            q->Enqueue(i);
        }
        return nullptr;
    }

    void *ConsumeThread1(void *arg) {
        auto q = (turbo::MPSCQueue<uint> *) arg;
        Consume(*q, true);
        return nullptr;
    }

    TEST(MPSCQueueTest, spsc_single_thread) {
        turbo::MPSCQueue<uint> q;
        for (uint i = 0; i < MAX_COUNT; ++i) {
            q.Enqueue(i);
        }
        Consume(q, false);
    }

    TEST(MPSCQueueTest, spsc_multi_thread) {
        turbo::MPSCQueue<uint> q;
        pthread_t produce_tid;
        ASSERT_EQ(0, pthread_create(&produce_tid, nullptr, ProduceThread, &q));
        pthread_t consume_tid;
        ASSERT_EQ(0, pthread_create(&consume_tid, nullptr, ConsumeThread1, &q));

        pthread_join(produce_tid, nullptr);
        pthread_join(consume_tid, nullptr);

    }

    std::atomic<uint> g_index(0);

    void *MultiProduceThread(void *arg) {
        auto q = (turbo::MPSCQueue<uint> *) arg;
        while (true) {
            uint i = g_index.fetch_add(1, std::memory_order_relaxed);
            if (i >= MAX_COUNT) {
                break;
            }
            q->Enqueue(i);
        }
        return nullptr;
    }

    std::mutex g_mutex;
    bool g_counts[MAX_COUNT];

    void Consume2(turbo::MPSCQueue<uint> &q) {
        uint empty_count = 0;
        uint count = 0;
        while (true) {
            uint d;
            if (!q.Dequeue(d)) {
                ASSERT_LT(empty_count++, (const uint) 10000);
                ::usleep(1 * 1000);
                continue;
            }
            ASSERT_LT(d, MAX_COUNT);
            {
                std::unique_lock lk(g_mutex);
                ASSERT_FALSE(g_counts[d]);
                g_counts[d] = true;
            }
            if (++count >= MAX_COUNT) {
                break;
            }
        }
    }

    void *ConsumeThread2(void *arg) {
        auto q = (turbo::MPSCQueue<uint> *) arg;
        Consume2(*q);
        return nullptr;
    }

    TEST(MPSCQueueTest, mpsc_multi_thread) {
        turbo::MPSCQueue<uint> q;

        int thread_num = 8;
        pthread_t threads[thread_num];
        for (int i = 0; i < thread_num; ++i) {
            ASSERT_EQ(0, pthread_create(&threads[i], nullptr, MultiProduceThread, &q));
        }

        pthread_t consume_tid;
        ASSERT_EQ(0, pthread_create(&consume_tid, nullptr, ConsumeThread2, &q));

        for (int i = 0; i < thread_num; ++i) {
            pthread_join(threads[i], nullptr);
        }
        pthread_join(consume_tid, nullptr);

    }


}