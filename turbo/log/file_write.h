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

#include <turbo/base/macros.h>
#include <turbo/strings/string_view.h>

namespace turbo {
    TURBO_NAMESPACE_BEGIN

    class FileWriter {
    public:
        virtual ~FileWriter() = default;

        // Initialize the file writer with the given path.
        // Returns 0 on success, or an error code on failure.
        // as the meaning that this function should open the
        // file ready for writing, and return 0 on success.
        virtual int initialize(std::string_view path) = 0;

        // reinitialize the file writer with the given path.
        // Returns 0 on success, or an error code on failure.
        // as the meaning that this function should open the
        // file ready for writing, and return 0 on success.
        // for that, some time, the file may be removed or
        // the file may be renamed by other process, such as
        //  some body remove it in the shell. avoid the log
        //  write to a black hole, we should reopen the file
        //  and write the log to the new file.
        virtual int reopen() = 0;

        // Write the given message to the file.
        virtual ssize_t write(std::string_view message) = 0;

        // Flush the file writer.
        virtual void flush() = 0;

        // Close the file writer.
        virtual void close() = 0;

        virtual std::string file_path() const = 0;

        virtual size_t file_size() const = 0;
    };

    TURBO_NAMESPACE_END
}  // namespace turbo
