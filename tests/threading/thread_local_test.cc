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
#include <errno.h>
#include <turbo/threading/thread_local.h>

namespace {

    TURBO_THREAD_LOCAL int *dummy = nullptr;
    const size_t NTHREAD = 8;
    static bool processed[NTHREAD + 1];
    static bool deleted[NTHREAD + 1];
    static bool register_check = false;

    struct YellObj {
        static int nc;
        static int nd;

        YellObj() {
            ++nc;
        }

        ~YellObj() {
            ++nd;
        }
    };

    int YellObj::nc = 0;
    int YellObj::nd = 0;

    static void check_global_variable() {
        EXPECT_TRUE(processed[NTHREAD]);
        EXPECT_TRUE(deleted[NTHREAD]);
        EXPECT_EQ(2, YellObj::nc);
        EXPECT_EQ(2, YellObj::nd);
    }

    class KumoThreadLocalTest : public ::testing::Test {
    protected:
        KumoThreadLocalTest() {
            if (!register_check) {
                register_check = true;
                turbo::thread_atexit(check_global_variable);
            }
        };

        virtual ~KumoThreadLocalTest() {};

        virtual void SetUp() {
        };

        virtual void TearDown() {
        };
    };


    TURBO_THREAD_LOCAL void *x;

    void *foo(void *arg) {
        x = arg;
        usleep(10000);
        printf("x=%p\n", x);
        return nullptr;
    }

    TEST_F(KumoThreadLocalTest, thread_local_keyword) {
        pthread_t th[2];
        pthread_create(&th[0], nullptr, foo, (void *) 1);
        pthread_create(&th[1], nullptr, foo, (void *) 2);
        pthread_join(th[0], nullptr);
        pthread_join(th[1], nullptr);
    }

    void *yell(void *) {
        YellObj *p = turbo::get_thread_local<YellObj>();
        EXPECT_TRUE(p);
        EXPECT_EQ(2, YellObj::nc);
        EXPECT_EQ(0, YellObj::nd);
        EXPECT_EQ(p, turbo::get_thread_local<YellObj>());
        EXPECT_EQ(2, YellObj::nc);
        EXPECT_EQ(0, YellObj::nd);
        return nullptr;
    }

    TEST_F(KumoThreadLocalTest, get_thread_local) {
        YellObj::nc = 0;
        YellObj::nd = 0;
        YellObj *p = turbo::get_thread_local<YellObj>();
        ASSERT_TRUE(p);
        ASSERT_EQ(1, YellObj::nc);
        ASSERT_EQ(0, YellObj::nd);
        ASSERT_EQ(p, turbo::get_thread_local<YellObj>());
        ASSERT_EQ(1, YellObj::nc);
        ASSERT_EQ(0, YellObj::nd);
        pthread_t th;
        ASSERT_EQ(0, pthread_create(&th, nullptr, yell, nullptr));
        pthread_join(th, nullptr);
        EXPECT_EQ(2, YellObj::nc);
        EXPECT_EQ(1, YellObj::nd);
    }

    void delete_dummy(void *arg) {
        *(bool *) arg = true;
        if (dummy) {
            delete dummy;
            dummy = nullptr;
        } else {
            printf("dummy is nullptr\n");
        }
    }

    void *proc_dummy(void *arg) {
        bool *p = (bool *) arg;
        *p = true;
        EXPECT_TRUE(dummy == nullptr);
        dummy = new int(p - processed);
        turbo::thread_atexit(delete_dummy, deleted + (p - processed));
        return nullptr;
    }

    TEST_F(KumoThreadLocalTest, sanity) {
        errno = 0;
        ASSERT_EQ(-1, turbo::thread_atexit(nullptr));
        ASSERT_EQ(EINVAL, errno);

        processed[NTHREAD] = false;
        deleted[NTHREAD] = false;
        proc_dummy(&processed[NTHREAD]);

        pthread_t th[NTHREAD];
        for (size_t i = 0; i < NTHREAD; ++i) {
            processed[i] = false;
            deleted[i] = false;
            ASSERT_EQ(0, pthread_create(&th[i], nullptr, proc_dummy, processed + i));
        }
        for (size_t i = 0; i < NTHREAD; ++i) {
            ASSERT_EQ(0, pthread_join(th[i], nullptr));
            ASSERT_TRUE(processed[i]);
            ASSERT_TRUE(deleted[i]);
        }
    }

    static std::ostringstream *oss = nullptr;

    inline std::ostringstream &get_oss() {
        if (oss == nullptr) {
            oss = new std::ostringstream;
        }
        return *oss;
    }

    void fun1() {
        get_oss() << "fun1" << std::endl;
    }

    void fun2() {
        get_oss() << "fun2" << std::endl;
    }

    void fun3(void *arg) {
        get_oss() << "fun3(" << (uintptr_t) arg << ")" << std::endl;
    }

    void fun4(void *arg) {
        get_oss() << "fun4(" << (uintptr_t) arg << ")" << std::endl;
    }

    static void check_result() {
        // Don't use gtest function since this function might be invoked when the main
        // thread quits, instances required by gtest functions are likely destroyed.
        assert(get_oss().str() == "fun4(0)\nfun3(2)\nfun2\n");
    }

    TEST_F(KumoThreadLocalTest, call_order_and_cancel) {
        turbo::thread_atexit_cancel(nullptr);
        turbo::thread_atexit_cancel(nullptr, nullptr);

        ASSERT_EQ(0, turbo::thread_atexit(check_result));

        ASSERT_EQ(0, turbo::thread_atexit(fun1));
        ASSERT_EQ(0, turbo::thread_atexit(fun1));
        ASSERT_EQ(0, turbo::thread_atexit(fun2));
        ASSERT_EQ(0, turbo::thread_atexit(fun3, (void *) 1));
        ASSERT_EQ(0, turbo::thread_atexit(fun3, (void *) 1));
        ASSERT_EQ(0, turbo::thread_atexit(fun3, (void *) 2));
        ASSERT_EQ(0, turbo::thread_atexit(fun4, nullptr));

        turbo::thread_atexit_cancel(nullptr);
        turbo::thread_atexit_cancel(nullptr, nullptr);
        turbo::thread_atexit_cancel(fun1);
        turbo::thread_atexit_cancel(fun3, nullptr);
        turbo::thread_atexit_cancel(fun3, (void *) 1);
    }

} // namespace
