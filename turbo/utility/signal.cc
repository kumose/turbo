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
#include <turbo/utility/signal.h>

namespace turbo {


    SignalHandler::SignalHandler() : SignalHandler(static_cast<Callback>(nullptr)) {}

    SignalHandler::SignalHandler(Callback cb) {
#if TURBO_HAVE_SIGACTION
        sa_.sa_handler = cb;
        sa_.sa_flags = 0;
        sigemptyset(&sa_.sa_mask);
#else
        cb_ = cb;
#endif
    }

#if TURBO_HAVE_SIGACTION

    SignalHandler::SignalHandler(const struct sigaction &sa) {
        memcpy(&sa_, &sa, sizeof(sa));
    }

#endif

    SignalHandler::Callback SignalHandler::callback() const {
#if TURBO_HAVE_SIGACTION
        return sa_.sa_handler;
#else
        return cb_;
#endif
    }

#if TURBO_HAVE_SIGACTION

    const struct sigaction &SignalHandler::action() const { return sa_; }

#endif

    turbo::Result<SignalHandler> get_signal_handler(int signum) {
#if TURBO_HAVE_SIGACTION
        struct sigaction sa;
        int ret = sigaction(signum, nullptr, &sa);
        if (ret != 0) {
            // TODO more detailed message using errno
            return turbo::io_error("sigaction call failed");
        }
        return SignalHandler(sa);
#else
        // To read the old handler, set the signal handler to something else temporarily
        SignalHandler::Callback cb = signal(signum, SIG_IGN);
        if (cb == SIG_ERR || signal(signum, cb) == SIG_ERR) {
            // TODO more detailed message using errno
            return turbo::io_error("signal call failed");
        }
        return SignalHandler(cb);
#endif
    }

    turbo::Result<SignalHandler> set_signal_handler(int signum, const SignalHandler &handler) {
#if TURBO_HAVE_SIGACTION
        struct sigaction old_sa;
        int ret = sigaction(signum, &handler.action(), &old_sa);
        if (ret != 0) {
            // TODO more detailed message using errno
            return turbo::io_error("sigaction call failed");
        }
        return SignalHandler(old_sa);
#else
        SignalHandler::Callback cb = signal(signum, handler.callback());
        if (cb == SIG_ERR) {
            // TODO more detailed message using errno
            return turbo::io_error("signal call failed");
        }
        return SignalHandler(cb);
#endif
        return turbo::OkStatus();
    }

    void reinstate_signal_handler(int signum, SignalHandler::Callback handler) {
#if !TURBO_HAVE_SIGACTION
        // Cannot report any errors from signal() (but there shouldn't be any)
        signal(signum, handler);
#endif
    }

    turbo::Status send_signal(int signum) {
        if (raise(signum) == 0) {
            return turbo::OkStatus();
        }
        if (errno == EINVAL) {
            return turbo::invalid_argument_error("Invalid signal number ", signum);
        }
        return io_error_with_errno_payload(errno, "Failed to raise signal");
    }

    turbo::Status send_signal_to_thread(int signum, uint64_t thread_id) {
#if defined(_WIN32)
        return turbo::unimplemented_error("Cannot send signal to specific thread on Windows");
#else
        // Have to use a C-style cast because pthread_t can be a pointer *or* integer type
        int r = pthread_kill((pthread_t) thread_id, signum);  // NOLINT readability-casting
        if (r == 0) {
            return turbo::OkStatus();
        }
        if (r == EINVAL) {
            return turbo::invalid_argument_error("Invalid signal number ", signum);
        }
        return io_error_with_errno_payload(r, "Failed to raise signal");
#endif
    }

}  // namespace turbo
