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

namespace turbo {

    // Returns true when fd is blocking, false otherwise.
    bool is_blocking(int fd);

    // Make file descriptor |fd| non-blocking
    // Returns 0 on success, -1 otherwise and errno is set (by fcntl)
    int make_non_blocking(int fd);

    // Make file descriptor |fd| blocking
    // Returns 0 on success, -1 otherwise and errno is set (by fcntl)
    int make_blocking(int fd);

    // Make file descriptor |fd| automatically closed during exec()
    // Returns 0 on success, -1 when error and errno is set (by fcntl)
    int make_close_on_exec(int fd);

    // Disable nagling on file descriptor |socket|.
    // Returns 0 on success, -1 when error and errno is set (by setsockopt)
    int make_no_delay(int socket);

}  // namespace turbo
