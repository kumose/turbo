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
#pragma once

#include <turbo/base/macros.h>
#include <turbo/utility/status.h>

namespace turbo {

    class TURBO_EXPORT SignalHandler {
    public:
        using Callback = void (*)(int);

        SignalHandler();

        explicit SignalHandler(Callback cb);

#if TURBO_HAVE_SIGACTION

        explicit SignalHandler(const struct sigaction &sa);

#endif

        Callback callback() const;

#if TURBO_HAVE_SIGACTION

        const struct sigaction &action() const;

#endif

    protected:
#if TURBO_HAVE_SIGACTION
        // Storing the full sigaction allows to restore the entire signal handling
        // configuration.
        struct sigaction sa_;
#else
        Callback cb_;
#endif
    };


    /// \brief Return the current handler for the given signal number.
    TURBO_EXPORT
    turbo::Result<SignalHandler> get_signal_handler(int signum);

    /// \brief Set a new handler for the given signal number.
    ///
    /// The old signal handler is returned.
    TURBO_EXPORT
    turbo::Result<SignalHandler> set_signal_handler(int signum, const SignalHandler& handler);

    /// \brief Reinstate the signal handler
    ///
    /// For use in signal handlers.  This is needed on platforms without sigaction()
    /// such as Windows, as the default signal handler is restored there as
    /// soon as a signal is raised.
    TURBO_EXPORT
    void reinstate_signal_handler(int signum, SignalHandler::Callback handler);

    /// \brief Send a signal to the current process
    ///
    /// The thread which will receive the signal is unspecified.
    TURBO_EXPORT
    turbo::Status send_signal(int signum);

    /// \brief Send a signal to the given thread
    ///
    /// This function isn't supported on Windows.
    TURBO_EXPORT
    turbo::Status send_signal_to_thread(int signum, uint64_t thread_id);

}  // namespace turbo
