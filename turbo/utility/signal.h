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
