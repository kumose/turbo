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

#include <turbo/log/logging.h>
#include <turbo/threading/simple_thread.h>
#include <turbo/threading/internal/thread_local.h>
#include <turbo/threading//waitable_event.h>
#include <gtest/gtest.h>

namespace turbo {

namespace {

class ThreadLocalTesterBase : public turbo::DelegateSimpleThreadPool::Delegate {
 public:
  typedef turbo::ThreadLocalPointer<ThreadLocalTesterBase> TLPType;

  ThreadLocalTesterBase(TLPType* tlp, turbo::WaitableEvent* done)
      : tlp_(tlp),
        done_(done) {
  }
  virtual ~ThreadLocalTesterBase() {}

 protected:
  TLPType* tlp_;
  turbo::WaitableEvent* done_;
};

class SetThreadLocal : public ThreadLocalTesterBase {
 public:
  SetThreadLocal(TLPType* tlp, turbo::WaitableEvent* done)
      : ThreadLocalTesterBase(tlp, done),
        val_(nullptr) {
  }
  virtual ~SetThreadLocal() {}

  void set_value(ThreadLocalTesterBase* val) { val_ = val; }

  virtual void Run() override {
    DKCHECK(!done_->IsSignaled());
    tlp_->Set(val_);
    done_->Signal();
  }

 private:
  ThreadLocalTesterBase* val_;
};

class GetThreadLocal : public ThreadLocalTesterBase {
 public:
  GetThreadLocal(TLPType* tlp, turbo::WaitableEvent* done)
      : ThreadLocalTesterBase(tlp, done),
        ptr_(nullptr) {
  }
  virtual ~GetThreadLocal() {}

  void set_ptr(ThreadLocalTesterBase** ptr) { ptr_ = ptr; }

  virtual void Run() override {
    DKCHECK(!done_->IsSignaled());
    *ptr_ = tlp_->Get();
    done_->Signal();
  }

 private:
  ThreadLocalTesterBase** ptr_;
};

}  // namespace

// In this test, we start 2 threads which will access a ThreadLocalPointer.  We
// make sure the default is nullptr, and the pointers are unique to the threads.
TEST(ThreadLocalTest, Pointer) {
  turbo::DelegateSimpleThreadPool tp1("ThreadLocalTest tp1", 1);
  turbo::DelegateSimpleThreadPool tp2("ThreadLocalTest tp1", 1);
  tp1.Start();
  tp2.Start();

  turbo::ThreadLocalPointer<ThreadLocalTesterBase> tlp;

  static ThreadLocalTesterBase* const kBogusPointer =
      reinterpret_cast<ThreadLocalTesterBase*>(0x1234);

  ThreadLocalTesterBase* tls_val;
  turbo::WaitableEvent done(true, false);

  GetThreadLocal getter(&tlp, &done);
  getter.set_ptr(&tls_val);

  // Check that both threads defaulted to nullptr.
  tls_val = kBogusPointer;
  done.Reset();
  tp1.AddWork(&getter);
  done.Wait();
  EXPECT_EQ(static_cast<ThreadLocalTesterBase*>(nullptr), tls_val);

  tls_val = kBogusPointer;
  done.Reset();
  tp2.AddWork(&getter);
  done.Wait();
  EXPECT_EQ(static_cast<ThreadLocalTesterBase*>(nullptr), tls_val);


  SetThreadLocal setter(&tlp, &done);
  setter.set_value(kBogusPointer);

  // Have thread 1 set their pointer value to kBogusPointer.
  done.Reset();
  tp1.AddWork(&setter);
  done.Wait();

  tls_val = nullptr;
  done.Reset();
  tp1.AddWork(&getter);
  done.Wait();
  EXPECT_EQ(kBogusPointer, tls_val);

  // Make sure thread 2 is still nullptr
  tls_val = kBogusPointer;
  done.Reset();
  tp2.AddWork(&getter);
  done.Wait();
  EXPECT_EQ(static_cast<ThreadLocalTesterBase*>(nullptr), tls_val);

  // Set thread 2 to kBogusPointer + 1.
  setter.set_value(kBogusPointer + 1);

  done.Reset();
  tp2.AddWork(&setter);
  done.Wait();

  tls_val = nullptr;
  done.Reset();
  tp2.AddWork(&getter);
  done.Wait();
  EXPECT_EQ(kBogusPointer + 1, tls_val);

  // Make sure thread 1 is still kBogusPointer.
  tls_val = nullptr;
  done.Reset();
  tp1.AddWork(&getter);
  done.Wait();
  EXPECT_EQ(kBogusPointer, tls_val);

  tp1.JoinAll();
  tp2.JoinAll();
}

TEST(ThreadLocalTest, Boolean) {
  {
    turbo::ThreadLocalBoolean tlb;
    EXPECT_FALSE(tlb.Get());

    tlb.Set(false);
    EXPECT_FALSE(tlb.Get());

    tlb.Set(true);
    EXPECT_TRUE(tlb.Get());
  }

  // Our slot should have been freed, we're all reset.
  {
    turbo::ThreadLocalBoolean tlb;
    EXPECT_FALSE(tlb.Get());
  }
}

}  // namespace turbo
