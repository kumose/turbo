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
