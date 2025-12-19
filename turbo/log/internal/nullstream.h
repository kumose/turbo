// Copyright (C) 2024 EA group inc.
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
//
// -----------------------------------------------------------------------------
// File: log/internal/nullstream.h
// -----------------------------------------------------------------------------
//
// Classes `NullStream`, `NullStreamMaybeFatal ` and `NullStreamFatal`
// implement a subset of the `LogMessage` API and are used instead when logging
// of messages has been disabled.

#ifndef TURBO_LOG_INTERNAL_NULLSTREAM_H_
#define TURBO_LOG_INTERNAL_NULLSTREAM_H_

#ifdef _WIN32
#include <cstdlib>
#else

#include <unistd.h>

#endif

#include <ios>
#include <ostream>

#include <turbo/base/macros.h>
#include <turbo/base/log_severity.h>
#include <turbo/strings/string_view.h>

namespace turbo {
    TURBO_NAMESPACE_BEGIN
    namespace log_internal {

        // A `NullStream` implements the API of `LogMessage` (a few methods and
        // `operator<<`) but does nothing.  All methods are defined inline so the
        // compiler can eliminate the whole instance and discard anything that's
        // streamed in.
        class NullStream {
        public:
            NullStream &at_location(std::string_view, int) { return *this; }

            template<typename SourceLocationType>
            NullStream &at_location(SourceLocationType) {
                return *this;
            }

            NullStream &no_prefix() { return *this; }

            NullStream &no_newline() { return *this; }

            NullStream &with_verbosity(int) { return *this; }

            template<typename TimeType>
            NullStream &with_timestamp(TimeType) {
                return *this;
            }

            template<typename Tid>
            NullStream &with_thread_id(Tid) {
                return *this;
            }

            template<typename LogEntryType>
            NullStream &with_metadata_from(const LogEntryType &) {
                return *this;
            }

            NullStream &with_perror() { return *this; }

            template<typename LogSinkType>
            NullStream &to_sink_also(LogSinkType *) {
                return *this;
            }

            template<typename LogSinkType>
            NullStream &to_sink_only(LogSinkType *) {
                return *this;
            }

            template<typename LogSinkType>
            NullStream &OutputToSink(LogSinkType *, bool) {
                return *this;
            }

            NullStream &internal_stream() { return *this; }
        };

        template<typename T>
        inline NullStream &operator<<(NullStream &str, const T &) {
            return str;
        }

        inline NullStream &operator<<(NullStream &str,
                                      std::ostream &(*)(std::ostream &os)) {
            return str;
        }

        inline NullStream &operator<<(NullStream &str,
                                      std::ios_base &(*)(std::ios_base &os)) {
            return str;
        }

        // `NullStreamMaybeFatal` implements the process termination semantics of
        // `LogMessage`, which is used for `DFATAL` severity and expression-defined
        // severity e.g. `KLOG(LEVEL(HowBadIsIt()))`.  Like `LogMessage`, it terminates
        // the process when destroyed if the passed-in severity equals `FATAL`.
        class NullStreamMaybeFatal final : public NullStream {
        public:
            explicit NullStreamMaybeFatal(turbo::LogSeverity severity)
                    : fatal_(severity == turbo::LogSeverity::kFatal) {}

            ~NullStreamMaybeFatal() {
                if (fatal_) {
                    _exit(1);
                }
            }

        private:
            bool fatal_;
        };

// `NullStreamFatal` implements the process termination semantics of
// `LogMessageFatal`, which means it always terminates the process.  `DFATAL`
// and expression-defined severity use `NullStreamMaybeFatal` above.
        class NullStreamFatal final : public NullStream {
        public:
            NullStreamFatal() = default;
            // TURBO_NORETURN doesn't seem to work on destructors with msvc, so
            // disable msvc's warning about the d'tor never returning.
#if defined(_MSC_VER) && !defined(__clang__)
#pragma warning(push)
#pragma warning(disable : 4722)
#endif

            TURBO_NORETURN ~NullStreamFatal() { _exit(1); }

#if defined(_MSC_VER) && !defined(__clang__)
#pragma warning(pop)
#endif
        };

    }  // namespace log_internal
    TURBO_NAMESPACE_END
}  // namespace turbo

#endif  // TURBO_LOG_INTERNAL_GLOBALS_H_
