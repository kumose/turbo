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

#include <turbo/base/macros.h>
#include <turbo/memory/scoped_ptr.h>
#include <turbo/threading/platform_thread.h>
#include <turbo/threading/simple_thread.h>
#include <turbo/threading/thread_collision_warner.h>
#include <gtest/gtest.h>

// '' : local class member function does not have a body
MSVC_PUSH_DISABLE_WARNING(4822)


#if defined(NDEBUG)

// Would cause a memory leak otherwise.
#undef DFAKE_MUTEX
#define DFAKE_MUTEX(obj) turbo::scoped_ptr<turbo::AsserterBase> obj

// In Release, we expect the AsserterBase::warn() to not happen.
#define EXPECT_NDEBUG_FALSE_DEBUG_TRUE EXPECT_FALSE

#else

// In Debug, we expect the AsserterBase::warn() to happen.
#define EXPECT_NDEBUG_FALSE_DEBUG_TRUE EXPECT_TRUE

#endif


namespace {

// This is the asserter used with ThreadCollisionWarner instead of the default
// DCheckAsserter. The method fail_state is used to know if a collision took
// place.
    class AssertReporter : public turbo::AsserterBase {
    public:
        AssertReporter()
                : failed_(false) {}

        virtual void warn() override {
            failed_ = true;
        }

        virtual ~AssertReporter() {}

        bool fail_state() const { return failed_; }

        void reset() { failed_ = false; }

    private:
        bool failed_;
    };

}  // namespace

TEST(ThreadCollisionTest, BookCriticalSection) {
    AssertReporter *local_reporter = new AssertReporter();

    turbo::ThreadCollisionWarner warner(local_reporter);
    EXPECT_FALSE(local_reporter->fail_state());

    {  // Pin section.
        DFAKE_SCOPED_LOCK_THREAD_LOCKED(warner);
        EXPECT_FALSE(local_reporter->fail_state());
        {  // Pin section.
            DFAKE_SCOPED_LOCK_THREAD_LOCKED(warner);
            EXPECT_FALSE(local_reporter->fail_state());
        }
    }
}

TEST(ThreadCollisionTest, ScopedRecursiveBookCriticalSection) {
    AssertReporter *local_reporter = new AssertReporter();

    turbo::ThreadCollisionWarner warner(local_reporter);
    EXPECT_FALSE(local_reporter->fail_state());

    {  // Pin section.
        DFAKE_SCOPED_RECURSIVE_LOCK(warner);
        EXPECT_FALSE(local_reporter->fail_state());
        {  // Pin section again (allowed by DFAKE_SCOPED_RECURSIVE_LOCK)
            DFAKE_SCOPED_RECURSIVE_LOCK(warner);
            EXPECT_FALSE(local_reporter->fail_state());
        }  // Unpin section.
    }  // Unpin section.

    // Check that section is not pinned
    {  // Pin section.
        DFAKE_SCOPED_LOCK(warner);
        EXPECT_FALSE(local_reporter->fail_state());
    }  // Unpin section.
}

TEST(ThreadCollisionTest, ScopedBookCriticalSection) {
    AssertReporter *local_reporter = new AssertReporter();

    turbo::ThreadCollisionWarner warner(local_reporter);
    EXPECT_FALSE(local_reporter->fail_state());

    {  // Pin section.
        DFAKE_SCOPED_LOCK(warner);
        EXPECT_FALSE(local_reporter->fail_state());
    }  // Unpin section.

    {  // Pin section.
        DFAKE_SCOPED_LOCK(warner);
        EXPECT_FALSE(local_reporter->fail_state());
        {
            // Pin section again (not allowed by DFAKE_SCOPED_LOCK)
            DFAKE_SCOPED_LOCK(warner);
                    EXPECT_NDEBUG_FALSE_DEBUG_TRUE(local_reporter->fail_state());
            // Reset the status of warner for further tests.
            local_reporter->reset();
        }  // Unpin section.
    }  // Unpin section.

    {
        // Pin section.
        DFAKE_SCOPED_LOCK(warner);
        EXPECT_FALSE(local_reporter->fail_state());
    }  // Unpin section.
}

TEST(ThreadCollisionTest, MTBookCriticalSectionTest) {
    class NonThreadSafeQueue {
    public:
        explicit NonThreadSafeQueue(turbo::AsserterBase *asserter)
                : push_pop_(asserter) {
        }

        void push(int value) {
            DFAKE_SCOPED_LOCK_THREAD_LOCKED(push_pop_);
        }

        int pop() {
            DFAKE_SCOPED_LOCK_THREAD_LOCKED(push_pop_);
            return 0;
        }

    private:
        DFAKE_MUTEX(push_pop_);

        TURBO_DISALLOW_COPY_AND_ASSIGN(NonThreadSafeQueue);
    };

    class QueueUser : public turbo::DelegateSimpleThread::Delegate {
    public:
        explicit QueueUser(NonThreadSafeQueue &queue)
                : queue_(queue) {}

        virtual void Run() override {
            queue_.push(0);
            queue_.pop();
        }

    private:
        NonThreadSafeQueue &queue_;
    };

    AssertReporter *local_reporter = new AssertReporter();

    NonThreadSafeQueue queue(local_reporter);

    QueueUser queue_user_a(queue);
    QueueUser queue_user_b(queue);

    turbo::DelegateSimpleThread thread_a(&queue_user_a, "queue_user_thread_a");
    turbo::DelegateSimpleThread thread_b(&queue_user_b, "queue_user_thread_b");

    thread_a.Start();
    thread_b.Start();

    thread_a.Join();
    thread_b.Join();

            EXPECT_NDEBUG_FALSE_DEBUG_TRUE(local_reporter->fail_state());
}

TEST(ThreadCollisionTest, MTScopedBookCriticalSectionTest) {
    // Queue with a 5 seconds push execution time, hopefuly the two used threads
    // in the test will enter the push at same time.
    class NonThreadSafeQueue {
    public:
        explicit NonThreadSafeQueue(turbo::AsserterBase *asserter)
                : push_pop_(asserter) {
        }

        void push(int value) {
            DFAKE_SCOPED_LOCK(push_pop_);
            turbo::sleep_for(turbo::Duration::seconds(5));
        }

        int pop() {
            DFAKE_SCOPED_LOCK(push_pop_);
            return 0;
        }

    private:
        DFAKE_MUTEX(push_pop_);

        TURBO_DISALLOW_COPY_AND_ASSIGN(NonThreadSafeQueue);
    };

    class QueueUser : public turbo::DelegateSimpleThread::Delegate {
    public:
        explicit QueueUser(NonThreadSafeQueue &queue)
                : queue_(queue) {}

        virtual void Run() override {
            queue_.push(0);
            queue_.pop();
        }

    private:
        NonThreadSafeQueue &queue_;
    };

    AssertReporter *local_reporter = new AssertReporter();

    NonThreadSafeQueue queue(local_reporter);

    QueueUser queue_user_a(queue);
    QueueUser queue_user_b(queue);

    turbo::DelegateSimpleThread thread_a(&queue_user_a, "queue_user_thread_a");
    turbo::DelegateSimpleThread thread_b(&queue_user_b, "queue_user_thread_b");

    thread_a.Start();
    thread_b.Start();

    thread_a.Join();
    thread_b.Join();

            EXPECT_NDEBUG_FALSE_DEBUG_TRUE(local_reporter->fail_state());
}

TEST(ThreadCollisionTest, MTSynchedScopedBookCriticalSectionTest) {
    // Queue with a 2 seconds push execution time, hopefuly the two used threads
    // in the test will enter the push at same time.
    class NonThreadSafeQueue {
    public:
        explicit NonThreadSafeQueue(turbo::AsserterBase *asserter)
                : push_pop_(asserter) {
        }

        void push(int value) {
            DFAKE_SCOPED_LOCK(push_pop_);
            turbo::sleep_for(turbo::Duration::seconds(2));
        }

        int pop() {
            DFAKE_SCOPED_LOCK(push_pop_);
            return 0;
        }

    private:
        DFAKE_MUTEX(push_pop_);

        TURBO_DISALLOW_COPY_AND_ASSIGN(NonThreadSafeQueue);
    };

    // This time the QueueUser class protects the non thread safe queue with
    // a lock.
    class QueueUser : public turbo::DelegateSimpleThread::Delegate {
    public:
        QueueUser(NonThreadSafeQueue &queue, std::mutex &lock)
                : queue_(queue),
                  lock_(lock) {}

        virtual void Run() override {
            {
                std::unique_lock auto_lock(lock_);
                queue_.push(0);
            }
            {
                std::unique_lock auto_lock(lock_);
                queue_.pop();
            }
        }

    private:
        NonThreadSafeQueue &queue_;
        std::mutex &lock_;
    };

    AssertReporter *local_reporter = new AssertReporter();

    NonThreadSafeQueue queue(local_reporter);

    std::mutex lock;

    QueueUser queue_user_a(queue, lock);
    QueueUser queue_user_b(queue, lock);

    turbo::DelegateSimpleThread thread_a(&queue_user_a, "queue_user_thread_a");
    turbo::DelegateSimpleThread thread_b(&queue_user_b, "queue_user_thread_b");

    thread_a.Start();
    thread_b.Start();

    thread_a.Join();
    thread_b.Join();

    EXPECT_FALSE(local_reporter->fail_state());
}

TEST(ThreadCollisionTest, MTSynchedScopedRecursiveBookCriticalSectionTest) {
    // Queue with a 2 seconds push execution time, hopefuly the two used threads
    // in the test will enter the push at same time.
    class NonThreadSafeQueue {
    public:
        explicit NonThreadSafeQueue(turbo::AsserterBase *asserter)
                : push_pop_(asserter) {
        }

        void push(int) {
            DFAKE_SCOPED_RECURSIVE_LOCK(push_pop_);
            bar();
            turbo::sleep_for(turbo::Duration::seconds(2));

        }

        int pop() {
            DFAKE_SCOPED_RECURSIVE_LOCK(push_pop_);
            return 0;
        }

        void bar() {
            DFAKE_SCOPED_RECURSIVE_LOCK(push_pop_);
        }

    private:
        DFAKE_MUTEX(push_pop_);

        TURBO_DISALLOW_COPY_AND_ASSIGN(NonThreadSafeQueue);
    };

    // This time the QueueUser class protects the non thread safe queue with
    // a lock.
    class QueueUser : public turbo::DelegateSimpleThread::Delegate {
    public:
        QueueUser(NonThreadSafeQueue &queue, std::mutex &lock)
                : queue_(queue),
                  lock_(lock) {}

        virtual void Run() override {
            {
                std::unique_lock auto_lock(lock_);
                queue_.push(0);
            }
            {
                std::unique_lock auto_lock(lock_);
                queue_.bar();
            }
            {
                std::unique_lock auto_lock(lock_);
                queue_.pop();
            }
        }

    private:
        NonThreadSafeQueue &queue_;
        std::mutex &lock_;
    };

    AssertReporter *local_reporter = new AssertReporter();

    NonThreadSafeQueue queue(local_reporter);

    std::mutex lock;

    QueueUser queue_user_a(queue, lock);
    QueueUser queue_user_b(queue, lock);

    turbo::DelegateSimpleThread thread_a(&queue_user_a, "queue_user_thread_a");
    turbo::DelegateSimpleThread thread_b(&queue_user_b, "queue_user_thread_b");

    thread_a.Start();
    thread_b.Start();

    thread_a.Join();
    thread_b.Join();

    EXPECT_FALSE(local_reporter->fail_state());
}
