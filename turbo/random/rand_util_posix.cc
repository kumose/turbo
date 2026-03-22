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
