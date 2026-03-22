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

#include <cstdio>
#include <turbo/log/file_write.h>

namespace turbo::log_internal {

    class AppendFile : public turbo::FileWriter {
    public:
        AppendFile() = default;

        ~AppendFile() override;

        // Initialize the file writer with the given path.
        // Returns 0 on success, or an error code on failure.
        // as the meaning that this function should open the
        // file ready for writing, and return 0 on success.
        int initialize(std::string_view path) override;

        // reinitialize the file writer with the given path.
        // Returns 0 on success, or an error code on failure.
        // as the meaning that this function should open the
        // file ready for writing, and return 0 on success.
        // for that, some time, the file may be removed or
        // the file may be renamed by other process, such as
        //  some body remove it in the shell. avoid the log
        //  write to a black hole, we should reopen the file
        //  and write the log to the new file.
        int reopen() override;

        // Write the given message to the file.
        ssize_t write(std::string_view message) override;

        // Flush the file writer.
        void flush() override;

        // Close the file writer.
        void close() override;

        size_t file_size() const override {
            return _written;
        }

        std::string file_path() const override {
            return _path;
        }

    private:
        std::string _path;
        char _buffer[64 * 1024];
        std::size_t _written = 0;
        FILE *_file = nullptr;

    };
}  // namespace turbo::log_internal