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
#include <turbo/files/filesystem.h>
#include <turbo/utility/status.h>

namespace turbo {

#if defined(OS_POSIX)

    // Read exactly |bytes| bytes from file descriptor |fd|, storing the result
    // in |buffer|. This function is protected against EINTR and partial reads.
    // Returns true iff |bytes| bytes have been successfully read from |fd|.
    TURBO_EXPORT bool read_from_fd(int fd, char *buffer, size_t bytes);

#endif  // OS_POSIX

    // Reads the file at |path| into |contents| and returns true on success and
    // false on error.  For security reasons, a |path| containing path traversal
    // components ('..') is treated as a read error and |contents| is set to empty.
    // In case of I/O error, |contents| holds the data that could be read from the
    // file before the error occurred.  When the file size exceeds |max_size|, the
    // function returns false with |contents| holding the file truncated to
    // |max_size|.
    // |contents| may be nullptr, in which case this function is useful for its side
    // effect of priming the disk cache (could be used for unit tests).
    TURBO_EXPORT turbo::Status read_file_to_string(const turbo::FilePath &path,
                                                   std::string *contents,
                                                   size_t max_size);

    // Reads the file at |path| into |contents| and returns true on success and
    // false on error.  For security reasons, a |path| containing path traversal
    // components ('..') is treated as a read error and |contents| is set to empty.
    // In case of I/O error, |contents| holds the data that could be read from the
    // file before the error occurred.
    // |contents| may be nullptr, in which case this function is useful for its side
    // effect of priming the disk cache (could be used for unit tests).
    TURBO_EXPORT turbo::Status read_file_to_string(const turbo::FilePath &path, std::string *contents);

    TURBO_EXPORT turbo::Result<std::string> read_file_to_string(const turbo::FilePath &path);
}  // namespace turbo
