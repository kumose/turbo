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
// File: log/log_sink.h
// -----------------------------------------------------------------------------
//
// This header declares the interface class `turbo::LogSink`.

#pragma once

// clang-format off
#if defined(__has_include)
    #if __has_include(<glog/logging.h>)
        #define GLOG_USE_GLOG_EXPORT
        #include <glog/logging.h>
        #define TURBO_HAS_GLOG 1
    #else
        #define TURBO_HAS_GLOG 0
    #endif  //__has_include(<glog/logging.h>)
#elif defined(TURBO_HAS_GLOG_H)
    #define GLOG_USE_GLOG_EXPORT
    #include <glog/logging.h>
    #define TURBO_HAS_GLOG 1
#endif  //__has_include
// clang-format on
#ifdef TURBO_HAS_GLOG
#include <turbo/base/log_severity.h>
#include <turbo/log/internal/log_message.h>

namespace turbo {
    constexpr LogSeverity as_turbo_log_level(::google::LogSeverity severity) {
        switch (severity) {
            case ::google::GLOG_FATAL:
                return LogSeverity::kFatal;
            case ::google::GLOG_ERROR:
                return LogSeverity::kError;
            case ::google::GLOG_WARNING:
                return LogSeverity::kWarning;
            case ::google::GLOG_INFO:
                return LogSeverity::kInfo;
            default:
                return LogSeverity::kInfo;
        }
    }

    struct BridgeFromGoogleLogging : ::google::LogSink {
        /// ext_message for testing to identify
        BridgeFromGoogleLogging(bool init_glog = true) {
            if (init_glog) {
                google::InitGoogleLogging("");
            }
            ::google::AddLogSink(this);
        }

        ~BridgeFromGoogleLogging() noexcept override {
            ::google::RemoveLogSink(this);
        }

        void send(
            ::google::LogSeverity severity,
            const char *full_filename,
            const char *base_filename,
            int line,
            const struct ::tm *pTime,
            const char *message,
            size_t message_len) override {
            auto level = as_turbo_log_level(severity);
            switch (level) {
                case turbo::LogSeverity::kFatal: {
                    log_internal::LogMessageFatal fatal(base_filename, line);
                    make_up_log_message(fatal, pTime, message, message_len);
                    break;
                }
                case turbo::LogSeverity::kError: {
                    log_internal::LogMessage err(base_filename, line, ::turbo::log_internal::LogMessage::ErrorTag{});
                    make_up_log_message(err, pTime, message, message_len);
                    break;
                }
                case turbo::LogSeverity::kWarning: {
                    log_internal::LogMessage warn(base_filename, line, ::turbo::log_internal::LogMessage::WarningTag{});
                    make_up_log_message(warn, pTime, message, message_len);
                    break;
                }
                case turbo::LogSeverity::kInfo: {
                    log_internal::LogMessage info(base_filename, line, ::turbo::log_internal::LogMessage::InfoTag{});
                    make_up_log_message(info, pTime, message, message_len);
                    break;
                }
                default: {
                    log_internal::LogMessage d(base_filename, line, ::turbo::log_internal::LogMessage::InfoTag{});
                    make_up_log_message(d, pTime, message, message_len);
                    break;
                }
            }
        }

    private:
        static void make_up_log_message(log_internal::LogMessage &msg, const struct ::tm *pTime,
                                        const char *message, size_t message_len) {
            turbo::Time t = turbo::Time::from_tm(*pTime, turbo::TimeZone::utc());
            msg.with_timestamp(t);
#ifdef PROXY_GLOG_TEST
            msg <<"from glog ";
#endif
            msg << std::string_view{message, message_len};
        }
    };
} // namespace turbo
#endif
