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
#include <algorithm>
#include <atomic>
#include <cerrno>
#include <limits>
#include <mutex>
#include <optional>
#include <sstream>
#include <thread>
#include <vector>

#include <signal.h>

#ifndef _WIN32

#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>

#endif

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>
#include <tests/testing/ktest_util.h>
#include <turbo/files/io_util.h>
#include <turbo/log/logging.h>
#include <turbo/utility/signal.h>

#ifdef WIN32
#define PIPE_WRITE _write
#define PIPE_READ _read
#else
#define PIPE_WRITE write
#define PIPE_READ read
#endif

namespace turbo {

#if !defined(_WIN32)

    void AssertChildExit(int child_pid, int expected_exit_status = 0) {
        ASSERT_GT(child_pid, 0);
        int child_status;
        int got_pid = waitpid(child_pid, &child_status, 0);
        ASSERT_EQ(got_pid, child_pid);
        if (WIFSIGNALED(child_status)) {
            FAIL() << "Child terminated by signal " << WTERMSIG(child_status);
        }
        if (!WIFEXITED(child_status)) {
            FAIL() << "Child didn't terminate normally?? Child status = " << child_status;
        }
        ASSERT_EQ(WEXITSTATUS(child_status), expected_exit_status);
    }

#endif

    class TURBO_EXPORT SignalHandlerGuard {
    public:
        typedef void (*Callback)(int);

        SignalHandlerGuard(int signum, Callback cb);

        SignalHandlerGuard(int signum, const turbo::SignalHandler &handler);

        ~SignalHandlerGuard();

    protected:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };

    struct SignalHandlerGuard::Impl {
        int signum_;
        SignalHandler old_handler_;

        Impl(int signum, const SignalHandler &handler)
                : signum_(signum), old_handler_(*set_signal_handler(signum, handler)) {}

        ~Impl() { TURBO_EXPECT_OK(set_signal_handler(signum_, old_handler_)); }
    };

    SignalHandlerGuard::SignalHandlerGuard(int signum, Callback cb)
            : SignalHandlerGuard(signum, SignalHandler(cb)) {}

    SignalHandlerGuard::SignalHandlerGuard(int signum, const SignalHandler &handler)
            : impl_(new Impl{signum, handler}) {}

    SignalHandlerGuard::~SignalHandlerGuard() = default;

    // Wait until predicate is true or timeout in seconds expires.
    void BusyWait(double seconds, std::function<bool()> predicate) {
        const double period = 0.001;
        for (int i = 0; !predicate() && i * period < seconds; ++i) {
            turbo::sleep_for(turbo::Duration::seconds(period));
        }
    }


    TEST(GetPageSize, Basics) {
        const auto page_size = get_page_size();
        ASSERT_GE(page_size, 4096);
        // It's a power of 2
        ASSERT_EQ((page_size - 1) & page_size, 0);
    }


    class TestFileDescriptor : public ::testing::Test {
    public:
        turbo::Result<int> NewFileDescriptor() {
            // Make a new fd by dup'ing C stdout (why not?)
            int new_fd = dup(1);
            if (new_fd < 0) {
                return io_error_with_errno_payload(errno, "Failed to dup() C stdout");
            }
            return new_fd;
        }

        void AssertValidFileDescriptor(int fd) {
            ASSERT_FALSE(file_is_closed(fd)) << "Not a valid file descriptor: " << fd;
        }

        void AssertInvalidFileDescriptor(int fd) {
            ASSERT_TRUE(file_is_closed(fd)) << "Unexpectedly valid file descriptor: " << fd;
        }
    };

    TEST_F(TestFileDescriptor, Basics) {
        int new_fd, new_fd2;

        // Default initialization
        FileDescriptor a;
        ASSERT_EQ(a.fd(), -1);
        ASSERT_TRUE(a.closed());
        ASSERT_OK(a.close());
        ASSERT_OK(a.close());

        // Assignment
        ASSERT_OK_AND_ASSIGN(new_fd, NewFileDescriptor());
        AssertValidFileDescriptor(new_fd);
        a = FileDescriptor(new_fd);
        ASSERT_FALSE(a.closed());
        ASSERT_GT(a.fd(), 2);
        ASSERT_OK(a.close());
        AssertInvalidFileDescriptor(new_fd);  // underlying fd was actually closed
        ASSERT_TRUE(a.closed());
        ASSERT_EQ(a.fd(), -1);
        ASSERT_OK(a.close());
        ASSERT_TRUE(a.closed());

        ASSERT_OK_AND_ASSIGN(new_fd, NewFileDescriptor());
        ASSERT_OK_AND_ASSIGN(new_fd2, NewFileDescriptor());

        // Move assignment
        FileDescriptor b(new_fd);
        FileDescriptor c(new_fd2);
        AssertValidFileDescriptor(new_fd);
        AssertValidFileDescriptor(new_fd2);
        c = std::move(b);
        ASSERT_TRUE(b.closed());
        ASSERT_EQ(b.fd(), -1);
        ASSERT_FALSE(c.closed());
        ASSERT_EQ(c.fd(), new_fd);
        AssertValidFileDescriptor(new_fd);
        AssertInvalidFileDescriptor(new_fd2);

        // Move constructor
        FileDescriptor d(std::move(c));
        ASSERT_TRUE(c.closed());
        ASSERT_EQ(c.fd(), -1);
        ASSERT_FALSE(d.closed());
        ASSERT_EQ(d.fd(), new_fd);
        AssertValidFileDescriptor(new_fd);

        // Detaching
        {
            FileDescriptor e(d.detach());
            ASSERT_TRUE(d.closed());
            ASSERT_EQ(d.fd(), -1);
            ASSERT_FALSE(e.closed());
            ASSERT_EQ(e.fd(), new_fd);
            AssertValidFileDescriptor(new_fd);
        }
        AssertInvalidFileDescriptor(new_fd);  // e was closed
    }

    class TestCreatePipe : public ::testing::Test {
    public:
        void TearDown() override { ASSERT_OK(pipe_.close()); }

    protected:
        Pipe pipe_;
    };

    TEST_F(TestCreatePipe, Blocking) {
        ASSERT_OK_AND_ASSIGN(pipe_, create_pipe());

        std::string buf("abcd");
        ASSERT_OK(file_write(pipe_.wfd.fd(), reinterpret_cast<const uint8_t *>(buf.data()),
                             buf.size()));
        buf = "xxxx";
        ASSERT_OK_AND_EQ(
                4, file_read(pipe_.rfd.fd(), reinterpret_cast<uint8_t *>(&buf[0]), buf.size()));
        ASSERT_EQ(buf, "abcd");
    }

    TEST_F(TestCreatePipe, NonBlocking) {
        ASSERT_OK_AND_ASSIGN(pipe_, create_pipe());
        ASSERT_OK(set_pipe_non_blocking(pipe_.rfd.fd()));
        ASSERT_OK(set_pipe_non_blocking(pipe_.wfd.fd()));

        std::string buf("abcd");
        ASSERT_OK(file_write(pipe_.wfd.fd(), reinterpret_cast<const uint8_t *>(buf.data()),
                             buf.size()));
        buf = "xxxx";
        ASSERT_OK_AND_EQ(
                4, file_read(pipe_.rfd.fd(), reinterpret_cast<uint8_t *>(&buf[0]), buf.size()));
        ASSERT_EQ(buf, "abcd");

        auto st =
                file_read(pipe_.rfd.fd(), reinterpret_cast<uint8_t *>(&buf[0]), buf.size()).status();
        ASSERT_EQ(turbo::is_io_error(st), true);
#ifdef _WIN32
        ASSERT_EQ(errno_from_status_payload(st), ERROR_NO_DATA);
#else
        ASSERT_EQ(errno_from_status_payload(st), EAGAIN);
#endif
    }

    class TestSelfPipe : public ::testing::Test {
    public:
        void SetUp() override {
            instance_ = this;
            ASSERT_OK_AND_ASSIGN(self_pipe_, SelfPipe::create(/*signal_safe=*/true));
        }

        void StartReading() {
            read_thread_ = std::thread([this]() { ReadUntilEof(); });
        }

        void FinishReading() { read_thread_.join(); }

        void TearDown() override {
            ASSERT_OK(self_pipe_->shutdown());
            if (read_thread_.joinable()) {
                read_thread_.join();
            }
            instance_ = nullptr;
        }

        turbo::Status ReadStatus() {
            std::lock_guard<std::mutex> lock(mutex_);
            return status_;
        }

        std::vector<uint64_t> ReadPayloads() {
            std::lock_guard<std::mutex> lock(mutex_);
            return payloads_;
        }

        void AssertPayloadsEventually(const std::vector<uint64_t> &expected) {
            BusyWait(1.0, [&]() { return ReadPayloads().size() == expected.size(); });
            ASSERT_EQ(ReadPayloads(), expected);
        }

    protected:
        void ReadUntilEof() {
            while (true) {
                auto maybe_payload = self_pipe_->wait();
                std::lock_guard<std::mutex> lock(mutex_);
                if (maybe_payload.ok()) {
                    payloads_.push_back(*maybe_payload);
                } else if (turbo::is_invalid_argument(maybe_payload.status())) {
                    // EOF
                    break;
                } else {
                    status_ = maybe_payload.status();
                    // Since we got an error, we may not be able to ever detect EOF,
                    // so bail out?
                    break;
                }
            }
        }

        static void HandleSignal(int signum) {
            instance_->signal_received_.store(signum);
            instance_->self_pipe_->send(123);
        }

        std::mutex mutex_;
        std::shared_ptr<SelfPipe> self_pipe_;
        std::thread read_thread_;
        std::vector<uint64_t> payloads_;
        turbo::Status status_;
        std::atomic<int> signal_received_;

        static TestSelfPipe *instance_;
    };

    TestSelfPipe *TestSelfPipe::instance_;

    TEST_F(TestSelfPipe, MakeAndShutdown) {}

    TEST_F(TestSelfPipe, WaitAndSend) {
        StartReading();
        turbo::sleep_for(turbo::Duration::milliseconds(1));
        AssertPayloadsEventually({});
        ASSERT_OK(ReadStatus());

        self_pipe_->send(123456789123456789ULL);
        self_pipe_->send(987654321987654321ULL);
        AssertPayloadsEventually({123456789123456789ULL, 987654321987654321ULL});
        ASSERT_OK(ReadStatus());
    }

    TEST_F(TestSelfPipe, SendAndWait) {

        self_pipe_->send(123456789123456789ULL);
        StartReading();
        turbo::sleep_for(turbo::Duration::milliseconds(1));
        self_pipe_->send(987654321987654321ULL);

        AssertPayloadsEventually({123456789123456789ULL, 987654321987654321ULL});
        ASSERT_OK(ReadStatus());
    }

    TEST_F(TestSelfPipe, WaitAndShutdown) {

        StartReading();
        turbo::sleep_for(turbo::Duration::milliseconds(1));
        ASSERT_OK(self_pipe_->shutdown());
        FinishReading();

        ASSERT_THAT(ReadPayloads(), testing::ElementsAre());
        ASSERT_OK(ReadStatus());
        ASSERT_OK(self_pipe_->shutdown());  // idempotent
    }

    TEST_F(TestSelfPipe, ShutdownAndWait) {

        self_pipe_->send(123456789123456789ULL);
        ASSERT_OK(self_pipe_->shutdown());
        StartReading();
        turbo::sleep_for(turbo::Duration::milliseconds(1));
        FinishReading();

        ASSERT_THAT(ReadPayloads(), testing::ElementsAre(123456789123456789ULL));
        ASSERT_OK(ReadStatus());
        ASSERT_OK(self_pipe_->shutdown());  // idempotent
    }

    TEST_F(TestSelfPipe, WaitAndSendFromSignal) {

        signal_received_.store(0);
        SignalHandlerGuard guard(SIGINT, &HandleSignal);

        StartReading();
        turbo::sleep_for(turbo::Duration::milliseconds(1));

        self_pipe_->send(456);
        ASSERT_OK(send_signal(SIGINT));  // will send 123
        self_pipe_->send(789);
        BusyWait(1.0, [&]() { return signal_received_.load() != 0; });
        ASSERT_EQ(signal_received_.load(), SIGINT);

        BusyWait(1.0, [&]() { return ReadPayloads().size() == 3; });
        ASSERT_THAT(ReadPayloads(), testing::UnorderedElementsAre(123, 456, 789));
        ASSERT_OK(ReadStatus());
    }

    TEST_F(TestSelfPipe, SendFromSignalAndWait) {

        signal_received_.store(0);
        SignalHandlerGuard guard(SIGINT, &HandleSignal);

        self_pipe_->send(456);
        ASSERT_OK(send_signal(SIGINT));  // will send 123
        self_pipe_->send(789);
        BusyWait(1.0, [&]() { return signal_received_.load() != 0; });
        ASSERT_EQ(signal_received_.load(), SIGINT);

        StartReading();

        BusyWait(1.0, [&]() { return ReadPayloads().size() == 3; });
        ASSERT_THAT(ReadPayloads(), testing::UnorderedElementsAre(123, 456, 789));
        ASSERT_OK(ReadStatus());
    }

#if !(defined(_WIN32) || defined(ADDRESS_SANITIZER) || \
      defined(THREAD_SANITIZER))
    TEST_F(TestSelfPipe, ForkSafety) {

        self_pipe_->send(123456789123456789ULL);

        auto child_pid = fork();
        if (child_pid == 0) {
            // Child: pipe is reinitialized and usable without interfering with parent
            self_pipe_->send(41ULL);
            StartReading();
            turbo::sleep_for(turbo::Duration::milliseconds(1));
            self_pipe_->send(42ULL);
            AssertPayloadsEventually({41ULL, 42ULL});

            self_pipe_.reset();
            std::exit(0);
        } else {
            // Parent: pipe is usable concurrently with child, data is read correctly
            StartReading();
            turbo::sleep_for(turbo::Duration::milliseconds(1));
            self_pipe_->send(987654321987654321ULL);

            AssertPayloadsEventually({123456789123456789ULL, 987654321987654321ULL});
            ASSERT_OK(ReadStatus());

            AssertChildExit(child_pid);
        }
    }

#endif


    class TestSendSignal : public ::testing::Test {
    protected:
        static std::atomic<int> signal_received_;

        static void HandleSignal(int signum) {
            reinstate_signal_handler(signum, &HandleSignal);
            signal_received_.store(signum);
        }
    };

    std::atomic<int> TestSendSignal::signal_received_;

    TEST_F(TestSendSignal, Generic) {
        signal_received_.store(0);
        SignalHandlerGuard guard(SIGINT, &HandleSignal);

        ASSERT_EQ(signal_received_.load(), 0);
        ASSERT_OK(send_signal(SIGINT));
        BusyWait(1.0, [&]() { return signal_received_.load() != 0; });
        ASSERT_EQ(signal_received_.load(), SIGINT);

        // Re-try (exercise ReinstateSignalHandler)
        signal_received_.store(0);
        ASSERT_OK(send_signal(SIGINT));
        BusyWait(1.0, [&]() { return signal_received_.load() != 0; });
        ASSERT_EQ(signal_received_.load(), SIGINT);
    }

    TEST_F(TestSendSignal, ToThread) {
        // Have to use a C-style cast because pthread_t can be a pointer *or* integer type
        uint64_t thread_id = (uint64_t) (pthread_self());  // NOLINT readability-casting
        signal_received_.store(0);
        SignalHandlerGuard guard(SIGINT, &HandleSignal);

        ASSERT_EQ(signal_received_.load(), 0);
        ASSERT_OK(send_signal_to_thread(SIGINT, thread_id));
        BusyWait(1.0, [&]() { return signal_received_.load() != 0; });

        ASSERT_EQ(signal_received_.load(), SIGINT);
    }

    TEST(Memory, GetRSS) {
#if defined(_WIN32)
        ASSERT_GT(get_current_rss(), 0);
#elif defined(__APPLE__)
        ASSERT_GT(get_current_rss(), 0);
#elif defined(__linux__)
        ASSERT_GT(get_current_rss(), 0);
#else
        ASSERT_EQ(get_current_rss(), 0);
#endif
    }

    TEST(Memory, TotalMemory) {
#if defined(_WIN32)
        ASSERT_GT(get_total_memory_bytes(), 0);
#elif defined(__APPLE__)
        ASSERT_GT(get_total_memory_bytes(), 0);
#elif defined(__linux__)
        ASSERT_GT(get_total_memory_bytes(), 0);
#else
        ASSERT_EQ(get_total_memory_bytes(), 0);
#endif
    }


}  // namespace turbo
