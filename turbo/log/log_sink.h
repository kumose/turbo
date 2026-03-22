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
// -----------------------------------------------------------------------------
// File: log/log_sink.h
// -----------------------------------------------------------------------------
//
// This header declares the interface class `turbo::LogSink`.

#pragma once
#include <turbo/base/macros.h>
#include <turbo/log/log_entry.h>

namespace turbo {

    // turbo::LogSink
    //
    // `turbo::LogSink` is an interface which can be extended to intercept and
    // process particular messages (with `KLOG.to_sink_only()` or
    // `KLOG.to_sink_also()`) or all messages (if registered with
    // `turbo::add_log_sink`).  Implementations must not take any locks that might be
    // held by the `KLOG` caller.
    class LogSink {
    public:
        virtual ~LogSink() = default;

        // LogSink::Send()
        //
        // `Send` is called synchronously during the log statement.  `Send` must be
        // thread-safe.
        //
        // It is safe to use `KLOG` within an implementation of `Send`.  `to_sink_only`
        // and `to_sink_also` are safe in general but can be used to create an infinite
        // loop if you try.
        virtual void Send(const turbo::LogEntry &entry) = 0;

        // LogSink::Flush()
        //
        // Sinks that buffer messages should override this method to flush the buffer
        // and return.  `Flush` must be thread-safe.
        virtual void Flush() {}

    protected:
        LogSink() = default;

        // Implementations may be copyable and/or movable.
        LogSink(const LogSink &) = default;

        LogSink &operator=(const LogSink &) = default;

    private:
        // https://lld.llvm.org/missingkeyfunction.html#missing-key-function
        virtual void KeyFunction() const final;  // NOLINT(readability/inheritance)
    };

}  // namespace turbo
