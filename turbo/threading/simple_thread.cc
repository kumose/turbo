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
#include <turbo/threading/simple_thread.h>

#include <turbo/log/logging.h>
#include <turbo/strings/numbers.h>
#include <turbo/threading/platform_thread.h>
#include <turbo/threading/thread_restrictions.h>

namespace turbo {

    SimpleThread::SimpleThread(const std::string &name_prefix)
            : name_prefix_(name_prefix), name_(name_prefix),
              thread_(), event_(true, false), tid_(0), joined_(false) {
    }

    SimpleThread::SimpleThread(const std::string &name_prefix,
                               const Options &options)
            : name_prefix_(name_prefix), name_(name_prefix), options_(options),
              thread_(), event_(true, false), tid_(0), joined_(false) {
    }

    SimpleThread::~SimpleThread() {
        DKCHECK(HasBeenStarted()) << "SimpleThread was never started.";
        DKCHECK(HasBeenJoined()) << "SimpleThread destroyed without being Join()ed.";
    }

    void SimpleThread::Start() {
        DKCHECK(!HasBeenStarted()) << "Tried to Start a thread multiple times.";
        bool success = PlatformThread::Create(options_.stack_size(), this, &thread_);
        DKCHECK(success);
        turbo::ThreadRestrictions::ScopedAllowWait allow_wait;
        event_.Wait();  // Wait for the thread to complete initialization.
    }

    void SimpleThread::Join() {
        DKCHECK(HasBeenStarted()) << "Tried to Join a never-started thread.";
        DKCHECK(!HasBeenJoined()) << "Tried to Join a thread multiple times.";
        PlatformThread::Join(thread_);
        joined_ = true;
    }

    bool SimpleThread::HasBeenStarted() {
        turbo::ThreadRestrictions::ScopedAllowWait allow_wait;
        return event_.IsSignaled();
    }

    void SimpleThread::ThreadMain() {
        tid_ = PlatformThread::CurrentId();
        // Construct our full name of the form "name_prefix_/TID".
        name_.push_back('/');
        name_.append(std::to_string(tid_));
        PlatformThread::SetName(name_.c_str());

        // We've initialized our new thread, signal that we're done to Start().
        event_.Signal();

        Run();
    }

    DelegateSimpleThread::DelegateSimpleThread(Delegate *delegate,
                                               const std::string &name_prefix)
            : SimpleThread(name_prefix),
              delegate_(delegate) {
    }

    DelegateSimpleThread::DelegateSimpleThread(Delegate *delegate,
                                               const std::string &name_prefix,
                                               const Options &options)
            : SimpleThread(name_prefix, options),
              delegate_(delegate) {
    }

    DelegateSimpleThread::~DelegateSimpleThread() {
    }

    void DelegateSimpleThread::Run() {
        DKCHECK(delegate_) << "Tried to call Run without a delegate (called twice?)";
        delegate_->Run();
        delegate_ = nullptr;
    }

    DelegateSimpleThreadPool::DelegateSimpleThreadPool(
            const std::string &name_prefix,
            int num_threads)
            : name_prefix_(name_prefix),
              num_threads_(num_threads),
              dry_(true, false) {
    }

    DelegateSimpleThreadPool::~DelegateSimpleThreadPool() {
        DKCHECK(threads_.empty());
        DKCHECK(delegates_.empty());
        DKCHECK(!dry_.IsSignaled());
    }

    void DelegateSimpleThreadPool::Start() {
        DKCHECK(threads_.empty()) << "Start() called with outstanding threads.";
        for (int i = 0; i < num_threads_; ++i) {
            DelegateSimpleThread *thread = new DelegateSimpleThread(this, name_prefix_);
            thread->Start();
            threads_.push_back(thread);
        }
    }

    void DelegateSimpleThreadPool::JoinAll() {
        DKCHECK(!threads_.empty()) << "JoinAll() called with no outstanding threads.";

        // Tell all our threads to quit their worker loop.
        AddWork(nullptr, num_threads_);

        // Join and destroy all the worker threads.
        for (int i = 0; i < num_threads_; ++i) {
            threads_[i]->Join();
            delete threads_[i];
        }
        threads_.clear();
        DKCHECK(delegates_.empty());
    }

    void DelegateSimpleThreadPool::AddWork(Delegate *delegate, int repeat_count) {
        std::unique_lock locked(lock_);
        for (int i = 0; i < repeat_count; ++i)
            delegates_.push(delegate);
        // If we were empty, signal that we have work now.
        if (!dry_.IsSignaled())
            dry_.Signal();
    }

    void DelegateSimpleThreadPool::Run() {
        Delegate *work = nullptr;

        while (true) {
            dry_.Wait();
            {
                std::unique_lock locked(lock_);
                if (!dry_.IsSignaled())
                    continue;

                DKCHECK(!delegates_.empty());
                work = delegates_.front();
                delegates_.pop();

                // Signal to any other threads that we're currently out of work.
                if (delegates_.empty())
                    dry_.Reset();
            }

            // A nullptr delegate pointer signals us to quit.
            if (!work)
                break;

            work->Run();
        }
    }

}  // namespace turbo
