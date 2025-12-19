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

    // Create an unix domain socket at `sockname' and listen to it.
    // If remove_previous_file is true or absent, remove previous file before
    // creating the socket.
    // Returns the file descriptor on success, -1 otherwise and errno is set.
    int unix_socket_listen(const char* sockname, bool remove_previous_file);
    int unix_socket_listen(const char* sockname);

    // Create an unix domain socket and connect it to another listening unix domain
    // socket at `sockname'.
    // Returns the file descriptor on success, -1 otherwise and errno is set.
    int unix_socket_connect(const char* sockname);

}  // namespace turbo
