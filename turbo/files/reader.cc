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

#include <turbo/files/reader.h>
#include <turbo/files/file_util.h>
#include <turbo/base/internal/eintr_wrapper.h>

namespace turbo {

#if defined(OS_POSIX)
    bool read_from_fd(int fd, char *buffer, size_t bytes) {
        size_t total_read = 0;
        while (total_read < bytes) {
            ssize_t bytes_read =
                    HANDLE_EINTR(read(fd, buffer + total_read, bytes - total_read));
            if (bytes_read <= 0)
                break;
            total_read += bytes_read;
        }
        return total_read == bytes;
    }
#endif // defined(OS_POSIX)

    turbo::Status read_file_to_string(const turbo::FilePath &fpath,
                             std::string *contents,
                             size_t max_size) {
        ErrnoCapture err;
        auto path = fpath.lexically_normal();
        if (contents) {
            contents->clear();
        }
        TURBO_MOVE_OR_RAISE(FILE *file, open_file(path, "rb"));
        char buf[1 << 16];
        size_t len;
        size_t size = 0;

        // Many files supplied in |path| have incorrect size (proc files etc).
        // Hence, the file is read sequentially as opposed to a one-shot read.
        while ((len = fread(buf, 1, sizeof(buf), file)) > 0) {
            if (contents)
                contents->append(buf, std::min(len, max_size - size));

            if ((max_size - size) < len) {
                err.capture();
                break;
            }

            size += len;
        }
        close_file(file).ignore_error();
        if(ferror(file)) {
            return turbo::errno_to_status(err(),"");
        }
        return turbo::OkStatus();
    }

    turbo::Status read_file_to_string(const turbo::FilePath &path, std::string *contents) {
        return read_file_to_string(path, contents, std::numeric_limits<size_t>::max());
    }

    TURBO_EXPORT turbo::Result<std::string> read_file_to_string(const turbo::FilePath &path) {
        std::string result;
        auto rs = read_file_to_string(path, &result);
        if(!rs.ok()) {
            return rs;
        }
        return result;
    }

}  // namespace turbo
