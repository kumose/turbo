//
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

#include <turbo/log/sinks/ansicolor_sink.h>
#include <turbo/strings/string_view.h>
#include <turbo/log/internal/fs_helper.h>

namespace turbo {

    AnsiColorSink::AnsiColorSink(FILE *file) : _file(file) {
        _color_active = log_internal::in_terminal(file) && log_internal::is_color_terminal();
    }

    std::array<std::string_view, static_cast<int>(LogSeverity::kFatal) + 1> colors_map = {
            AnsiColorSink::green,  // kInfo
            AnsiColorSink::yellow_bold, // kWarning
            AnsiColorSink::red_bold, // kError
            AnsiColorSink::bold_on_red, // kFatal
    };

    void AnsiColorSink::set_level_color(const LogSeverity severity, const std::string_view color) {
        colors_map[static_cast<int>(severity)] = color;
    }

    void AnsiColorSink::Send(const LogEntry &entry) {
        std::lock_guard<std::mutex> lock(_mutex);
        std::string_view log_data = entry.newline() ? entry.text_message_with_prefix_and_newline() : entry.text_message_with_prefix();
        if (!_color_active) {
            if (entry.log_severity() != LogSeverity::kFatal) {
                ::fwrite(log_data.data(), 1, log_data.size(), _file);
            } else {
                if (entry.stacktrace().size() > 0) {
                    ::fwrite(log_data.data(), 1, log_data.size(), _file);
                    ::fwrite(entry.stacktrace().data(), 1, entry.stacktrace().size(), _file);
                }
            }
            return;
        }

        if (entry.log_severity() != LogSeverity::kFatal) {
            auto color = colors_map[static_cast<int>(entry.log_severity())];
            ::fwrite(color.data(), 1, color.size(), _file);
            ::fwrite(log_data.data(), 1, log_data.size(), _file);
            ::fwrite(reset.data(), 1, reset.size(), _file);
        } else {
            if (entry.stacktrace().size() > 0) {
                auto color = colors_map[static_cast<int>(entry.log_severity())];
                ::fwrite(color.data(), 1, color.size(), _file);
                ::fwrite(log_data.data(), 1, log_data.size(), _file);
                ::fwrite(reset.data(), 1, reset.size(), _file);
                ::fwrite(entry.stacktrace().data(), 1, entry.stacktrace().size(), _file);
            }
        }
    }

    void AnsiColorSink::Flush() {
        std::lock_guard<std::mutex> lock(_mutex);
        ::fflush(_file);
    }

}  // namespace turbo
