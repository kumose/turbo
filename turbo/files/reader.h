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
