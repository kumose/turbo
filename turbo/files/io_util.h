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

#include <turbo/utility/status.h>
#include <turbo/base/macros.h>
#include <turbo/files/internal/windows_fixup.h>
#include <turbo/files/filesystem.h>

namespace turbo {

    TURBO_EXPORT turbo::Status file_close(int fd);

    /// The underlying file descriptor is automatically closed on destruction.
    /// Moving is supported with well-defined semantics.
    /// Furthermore, closing is idempotent.
    class TURBO_EXPORT FileDescriptor {
    public:
        FileDescriptor() = default;

        explicit FileDescriptor(int fd) : fd_(fd) {}

        FileDescriptor(FileDescriptor &&) noexcept;

        FileDescriptor &operator=(FileDescriptor &&) noexcept;

        ~FileDescriptor();

        turbo::Status close();

        /// May return -1 if closed or default-initialized
        int fd() const { return fd_.load(); }

        /// Detach and return the underlying file descriptor
        int detach();

        bool closed() const { return fd_.load() == -1; }

    protected:
        static void close_from_destructor(int fd);

        std::atomic<int> fd_{-1};
    };

    /// Open a file for reading and return a file descriptor.
    TURBO_EXPORT
    turbo::Result<FileDescriptor> file_open_readable(const turbo::FilePath& file_name);

    /// Open a file for writing and return a file descriptor.
    TURBO_EXPORT
    turbo::Result<FileDescriptor> file_open_writable(const turbo::FilePath& file_name,
                                                   bool write_only = true, bool truncate = true,
                                                   bool append = false);

    /// Read from current file position.  Return number of bytes read.
    TURBO_EXPORT
    turbo::Result<int64_t> file_read(int fd, uint8_t* buffer, int64_t nbytes);

    /// Read from given file position.  Return number of bytes read.
    TURBO_EXPORT
    turbo::Result<int64_t> file_read_at(int fd, uint8_t* buffer, int64_t position, int64_t nbytes);


    TURBO_EXPORT
    turbo::Status file_write(int fd, const uint8_t* buffer, size_t nbytes);

    TURBO_EXPORT
    turbo::Result<int64_t> file_write_at(int fd, const uint8_t* buffer, int64_t position, int64_t nbytes);


    TURBO_EXPORT
    turbo::Status file_truncate(int fd, const int64_t size);


    TURBO_EXPORT
    turbo::Status file_seek(int fd, int64_t pos);

    TURBO_EXPORT
    turbo::Status file_seek(int fd, int64_t pos, int whence);

    TURBO_EXPORT
    turbo::Result<int64_t> file_tell(int fd);

    TURBO_EXPORT
    turbo::Result<int64_t> file_get_size(int fd);

    TURBO_EXPORT bool file_is_closed(int fd);


    struct Pipe {
        FileDescriptor rfd;
        FileDescriptor wfd;

        turbo::Status close() { return rfd.close() & wfd.close(); }
    };

    TURBO_EXPORT
    turbo::Result<Pipe> create_pipe();

    TURBO_EXPORT
    turbo::Status set_pipe_non_blocking(int fd);

    class TURBO_EXPORT SelfPipe {
    public:
        static turbo::Result<std::shared_ptr<SelfPipe>> create(bool signal_safe);
        virtual ~SelfPipe();

        /// \brief Wait for a wakeup.
        ///
        /// StatusBuilder::Invalid is returned if the pipe has been shutdown.
        /// Otherwise the next sent payload is returned.
        virtual turbo::Result<uint64_t> wait() = 0;

        /// \brief Wake up the pipe by sending a payload.
        ///
        /// This method is async-signal-safe if `signal_safe` was set to true.
        virtual void send(uint64_t payload) = 0;

        /// \brief Wake up the pipe and shut it down.
        virtual turbo::Status shutdown() = 0;
    };


    struct MemoryRegion {
        void* addr;
        size_t size;
    };

    TURBO_EXPORT int64_t get_page_size();

    /// \brief Get the current memory used by the current process in bytes
    ///
    /// This function supports Windows, Linux, and Mac and will return 0 otherwise
    TURBO_EXPORT
    int64_t get_current_rss();

    /// \brief Get the total memory available to the system in bytes
    ///
    /// This function supports Windows, Linux, and Mac and will return 0 otherwise
    TURBO_EXPORT
    int64_t get_total_memory_bytes();


    TURBO_EXPORT
    turbo::Status memory_map_remap(void* addr, size_t old_size, size_t new_size, int fildes,
                                 void** new_addr);
    TURBO_EXPORT
    turbo::Status memory_advise_will_need(const std::vector<MemoryRegion>& regions);

}  // namespace turbo
