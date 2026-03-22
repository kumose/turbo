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
//

#pragma once

#include <turbo/log/log_sink.h>
#include <deque>
#include <memory>
#include <mutex>
#include <turbo/log/internal/append_file.h>
#include <turbo/times/time.h>
#include <turbo/container/circular_queue.h>
#include <turbo/log/log_sink_registry.h>
#include <turbo/log/log_filename.h>

namespace turbo {

    class RotatingFileSink : public LogSink {
    public:
        RotatingFileSink(std::string_view base_filename,
                         std::size_t max_size,
                         std::size_t max_files = 0,
                         int check_interval_s = 60);

        ~RotatingFileSink() override;

        void Send(const LogEntry &entry) override;

        void Flush() override;

    private:
        void do_rotate(turbo::Time stmp);

        bool rename_file(const std::string &src_filename, const std::string &target_filename);

    private:
        const std::string _raw_base_filename;
        const BaseFilename _base_filename;
        std::size_t max_size_;
        std::size_t max_files_;
        int _check_interval_s;
        turbo::Time _next_check_time;
        std::unique_ptr<FileWriter> _file_writer;
        std::mutex _mutex;

    };
}  // namespace  turbo
