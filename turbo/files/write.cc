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

#include <turbo/files/writer.h>
#include <turbo/files/io_util.h>
#include <turbo/threading/thread_restrictions.h>

namespace turbo {

    turbo::Status write_to_file(const turbo::FilePath &filename, const uint8_t *data, size_t size) {
        ThreadRestrictions::AssertIOAllowed();
        TURBO_MOVE_OR_RAISE(auto fd, file_open_writable(filename));
        return file_write(fd.fd(), data, size);
    }

    turbo::Status write_to_file(const turbo::FilePath &filename, std::string_view data) {
        return write_to_file(filename, reinterpret_cast<const uint8_t *>(data.data()), data.size());
    }

}  // namespace turbo
