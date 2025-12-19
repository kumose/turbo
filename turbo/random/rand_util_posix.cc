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

#include <turbo/random/rand_util.h>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <turbo/files/file_util.h>
#include <turbo/files/reader.h>
#include <turbo/memory/lazy_instance.h>
#include <turbo/log/logging.h>

namespace {

    // We keep the file descriptor for /dev/urandom around so we don't need to
    // reopen it (which is expensive), and since we may not even be able to reopen
    // it if we are later put in a sandbox. This class wraps the file descriptor so
    // we can use LazyInstance to handle opening it on the first access.
    class URandomFd {
    public:
        URandomFd() : fd_(open("/dev/urandom", O_RDONLY)) {
            DKCHECK_GE(fd_, 0) << "Cannot open /dev/urandom: " << errno;
        }

        ~URandomFd() { close(fd_); }

        int fd() const { return fd_; }

    private:
        const int fd_;
    };

    turbo::LazyInstance<URandomFd>::Leaky g_urandom_fd = LAZY_INSTANCE_INITIALIZER;

}  // namespace

namespace turbo {

// NOTE: This function must be cryptographically secure. http://crbug.com/140076
    uint64_t RandUint64() {
        uint64_t number;
        RandBytes(&number, sizeof(number));
        return number;
    }

    void RandBytes(void *output, size_t output_length) {
        const int urandom_fd = g_urandom_fd.Pointer()->fd();
        const bool success =
                read_from_fd(urandom_fd, static_cast<char *>(output), output_length);
        KCHECK(success);
    }

    int GetUrandomFD(void) {
        return g_urandom_fd.Pointer()->fd();
    }

}  // namespace turbo
