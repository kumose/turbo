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
#include <turbo/files/io_util.h>
#include <turbo/base/checked_cast.h>
#include <turbo/base/internal/eintr_wrapper.h>
#include <turbo/bootstrap/atfork.h>
#include <turbo/log/logging.h>

// define max read/write count
#ifdef _WIN32
#define TURBO_MAX_IO_CHUNKSIZE INT32_MAX
#else

#ifdef __APPLE__
// due to macOS bug, we need to set read/write max
#define TURBO_MAX_IO_CHUNKSIZE INT32_MAX
#else
// see notes on Linux read/write manpage
#define TURBO_MAX_IO_CHUNKSIZE 0x7ffff000
#endif

#endif

#ifdef _WIN32
#include <turbo/files/internal/mman.h>
#undef Realloc
#undef Free
#else  // POSIX-like platforms

#include <sys/mman.h>
#include <unistd.h>

#endif

#ifdef _WIN32
#include <psapi.h>

#elif __APPLE__
#include <mach/mach.h>
#include <sys/sysctl.h>

#elif __linux__
#include <sys/sysinfo.h>
#include <fstream>
#endif

#ifdef _WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

namespace turbo {

    //
    // Closing files
    //

    turbo::Status file_close(int fd) {
        int ret;

#if defined(_WIN32)
        ret = static_cast<int>(_close(fd));
#else
        ret = static_cast<int>(close(fd));
#endif

        if (ret == -1) {
            return turbo::io_error("error closing file");
        }
        return turbo::OkStatus();
    }

    //
    // Creating and destroying file descriptors
    //

    FileDescriptor::FileDescriptor(FileDescriptor &&other) noexcept: fd_(other.fd_.exchange(-1)) {}

    FileDescriptor &FileDescriptor::operator=(FileDescriptor &&other) noexcept {
        int old_fd = fd_.exchange(other.fd_.exchange(-1));
        if (old_fd != -1) {
            close_from_destructor(old_fd);
        }
        return *this;
    }

    void FileDescriptor::close_from_destructor(int fd) {
        TURBO_WARN_NOT_OK(file_close(fd), "Failed to close file descriptor");
    }

    FileDescriptor::~FileDescriptor() {
        int fd = fd_.load();
        if (fd != -1) {
            close_from_destructor(fd);
        }
    }

    turbo::Status FileDescriptor::close() {
        int fd = fd_.exchange(-1);
        if (fd != -1) {
            return file_close(fd);
        }
        return turbo::OkStatus();
    }

    int FileDescriptor::detach() { return fd_.exchange(-1); }

    static turbo::Result<int64_t> lseek64_compat(int fd, int64_t pos, int whence) {
#if defined(_WIN32)
        int64_t ret = _lseeki64(fd, pos, whence);
#else
        int64_t ret = lseek(fd, pos, whence);
#endif
        if (ret == -1) {
            return turbo::io_error("lseek failed");
        }
        return ret;
    }


    turbo::Result<FileDescriptor> file_open_readable(const turbo::FilePath &file_name) {
        FileDescriptor fd;
#if defined(_WIN32)
        HANDLE file_handle = CreateFileW(file_name.native().c_str(), GENERIC_READ,
                                   FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (file_handle == INVALID_HANDLE_VALUE) {
            return io_error_with_errno_payload(GetLastError(), "Failed to open local file '",
                                   file_name.to_string(), "'");
        }
        int ret = _open_osfhandle(reinterpret_cast<intptr_t>(file_handle),
                                _O_RDONLY | _O_BINARY | _O_NOINHERIT);
        if (ret == -1) {
            CloseHandle(file_handle);
            return io_error_with_errno_payload(errno, "Failed to open local file '", file_name.to_string(),
                                    "'");
        }
        fd = FileDescriptor(ret);
#else
        int ret = open(file_name.native().c_str(), O_RDONLY);
        if (ret < 0) {
            return io_error_with_errno_payload(errno, "Failed to open local file '", file_name.string(),
                                               "'");
        }
        // open(O_RDONLY) succeeds on directories, check for it
        fd = FileDescriptor(ret);
        struct stat st;
        ret = fstat(fd.fd(), &st);
        if (ret == 0 && S_ISDIR(st.st_mode)) {
            return turbo::io_error("Cannot open for reading: path '", file_name.string(),
                                   "' is a directory");
        }
#endif

        return fd;
    }


    turbo::Result<FileDescriptor> file_open_writable(const turbo::FilePath &file_name,
                                                     bool write_only, bool truncate, bool append) {
        FileDescriptor fd;

#if defined(_WIN32)
        DWORD desired_access = GENERIC_WRITE;
      DWORD share_mode = FILE_SHARE_READ | FILE_SHARE_WRITE;
      DWORD creation_disposition = OPEN_ALWAYS;

      if (truncate) {
        creation_disposition = CREATE_ALWAYS;
      }

      if (!write_only) {
        desired_access |= GENERIC_READ;
      }

      HANDLE file_handle =
          CreateFileW(file_name.ToNative().c_str(), desired_access, share_mode, NULL,
                      creation_disposition, FILE_ATTRIBUTE_NORMAL, NULL);
      if (file_handle == INVALID_HANDLE_VALUE) {
        return io_error_with_errno_payload(GetLastError(), "Failed to open local file '",
                                   file_name.to_string(), "'");
      }

      int ret = _open_osfhandle(reinterpret_cast<intptr_t>(file_handle),
                                _O_RDONLY | _O_BINARY | _O_NOINHERIT);
      if (ret == -1) {
        CloseHandle(file_handle);
        return io_error_with_errno_payload(errno, "Failed to open local file '", file_name.to_string(),
                                "'");
      }
      fd = FileDescriptor(ret);
#else
        int oflag = O_CREAT;

        if (truncate) {
            oflag |= O_TRUNC;
        }
        if (append) {
            oflag |= O_APPEND;
        }

        if (write_only) {
            oflag |= O_WRONLY;
        } else {
            oflag |= O_RDWR;
        }

        int ret = open(file_name.native().c_str(), oflag, 0666);
        if (ret == -1) {
            return io_error_with_errno_payload(errno, "Failed to open local file '", file_name.string(),
                                               "'");
        }
        fd = FileDescriptor(ret);
#endif

        if (append) {
            // Seek to end, as O_APPEND does not necessarily do it
            TURBO_RETURN_NOT_OK(lseek64_compat(fd.fd(), 0, SEEK_END));
        }
        return fd;
    }


    turbo::Result<int64_t> file_read(int fd, uint8_t *buffer, int64_t nbytes) {
#if defined(_WIN32)
        HANDLE handle = reinterpret_cast<HANDLE>(_get_osfhandle(fd));
#endif
        int64_t total_bytes_read = 0;

        while (total_bytes_read < nbytes) {
            const int64_t chunksize =
                    std::min(static_cast<int64_t>(TURBO_MAX_IO_CHUNKSIZE), nbytes - total_bytes_read);
            int64_t bytes_read = 0;
#if defined(_WIN32)
            DWORD dwBytesRead = 0;
            if (!ReadFile(handle, buffer, static_cast<uint32_t>(chunksize), &dwBytesRead,
                          nullptr)) {
              auto errnum = GetLastError();
              // Return a normal EOF when the write end of a pipe was closed
              if (errnum != ERROR_HANDLE_EOF && errnum != ERROR_BROKEN_PIPE) {
                return io_error_with_errno_payload(GetLastError(), "Error reading bytes from file");
              }
            }
            bytes_read = dwBytesRead;
#else
            bytes_read = static_cast<int64_t>(read(fd, buffer, static_cast<size_t>(chunksize)));
            if (bytes_read == -1) {
                if (errno == EINTR) {
                    continue;
                }
                return io_error_with_errno_payload(errno, "Error reading bytes from file");
            }
#endif

            if (bytes_read == 0) {
                // EOF
                break;
            }
            buffer += bytes_read;
            total_bytes_read += bytes_read;
        }
        return total_bytes_read;
    }

    static inline int64_t pread_compat(int fd, void *buf, int64_t nbytes, int64_t pos) {
#if defined(_WIN32)
        HANDLE handle = reinterpret_cast<HANDLE>(_get_osfhandle(fd));
        DWORD dwBytesRead = 0;
        OVERLAPPED overlapped = {};
        overlapped.Offset = static_cast<uint32_t>(pos);
        overlapped.OffsetHigh = static_cast<uint32_t>(pos >> 32);

        // Note: ReadFile() will update the file position
        BOOL bRet =
          ReadFile(handle, buf, static_cast<uint32_t>(nbytes), &dwBytesRead, &overlapped);
        if (bRet || GetLastError() == ERROR_HANDLE_EOF) {
            return dwBytesRead;
        } else {
            return -1;
        }
#else
        int64_t ret;
        do {
            ret = static_cast<int64_t>(
                    pread(fd, buf, static_cast<size_t>(nbytes), static_cast<off_t>(pos)));
        } while (ret == -1 && errno == EINTR);
        return ret;
#endif
    }

    turbo::Result<int64_t> file_read_at(int fd, uint8_t *buffer, int64_t position, int64_t nbytes) {
        int64_t bytes_read = 0;

        while (bytes_read < nbytes) {
            int64_t chunksize =
                    std::min(static_cast<int64_t>(TURBO_MAX_IO_CHUNKSIZE), nbytes - bytes_read);
            int64_t ret = pread_compat(fd, buffer, chunksize, position);

            if (ret == -1) {
                return io_error_with_errno_payload(errno, "Error reading bytes from file");
            }
            if (ret == 0) {
                // EOF
                break;
            }
            buffer += ret;
            position += ret;
            bytes_read += ret;
        }
        return bytes_read;
    }

    //
    // Writing data
    //

    turbo::Status file_write(int fd, const uint8_t *buffer, size_t nbytes) {
        int64_t bytes_written = 0;

        while (bytes_written < nbytes) {
            const int64_t chunksize =
                    std::min(static_cast<size_t>(TURBO_MAX_IO_CHUNKSIZE), nbytes - bytes_written);
#if defined(_WIN32)
            int64_t ret = static_cast<int64_t>(
            _write(fd, buffer + bytes_written, static_cast<uint32_t>(chunksize)));
#else
            int64_t ret = static_cast<int64_t>(
                    write(fd, buffer + bytes_written, static_cast<size_t>(chunksize)));
            if (ret == -1 && errno == EINTR) {
                continue;
            }
#endif

            if (ret == -1) {
                return io_error_with_errno_payload(errno, "Error writing bytes to file");
            }
            bytes_written += ret;
        }

        return turbo::OkStatus();
    }

    turbo::Result<int64_t> file_write_at(int fd, const uint8_t *buffer, int64_t position, int64_t nbytes) {

        int bytes_written = 0;
        int rv;
        do {
            rv = HANDLE_EINTR(pwrite(fd, buffer + bytes_written,
                                     nbytes - bytes_written, position + bytes_written));
            if (rv == -1) {
                return io_error_with_errno_payload(errno, "Error writing bytes to file");
            }

            if (rv == 0)
                break;

            bytes_written += rv;
        } while (bytes_written < nbytes);
        return bytes_written;
    }

    turbo::Status file_truncate(int fd, const int64_t size) {
        int ret, errno_actual;

#ifdef _WIN32
        errno_actual = _chsize_s(fd, static_cast<size_t>(size));
  ret = errno_actual == 0 ? 0 : -1;
#else
        ret = ftruncate(fd, static_cast<size_t>(size));
        errno_actual = errno;
#endif

        if (ret == -1) {
            return io_error_with_errno_payload(errno_actual, "Error writing bytes to file");
        }
        return turbo::OkStatus();
    }

    turbo::Status file_seek(int fd, int64_t pos, int whence) {
        return lseek64_compat(fd, pos, whence).status();
    }

    turbo::Status file_seek(int fd, int64_t pos) { return file_seek(fd, pos, SEEK_SET); }

    turbo::Result<int64_t> file_tell(int fd) {
#if defined(_WIN32)
        int64_t current_pos = _telli64(fd);
        if (current_pos == -1) {
            return turbo::io_error("_telli64 failed");
        }
        return current_pos;
#else
        return lseek64_compat(fd, 0, SEEK_CUR);
#endif
    }


    turbo::Result<int64_t> file_get_size(int fd) {
#if defined(_WIN32)
        struct __stat64 st;
#else
        struct stat st;
#endif
        st.st_size = -1;

#if defined(_WIN32)
        int ret = _fstat64(fd, &st);
#else
        int ret = fstat(fd, &st);
#endif

        if (ret == -1) {
            return turbo::io_error("error stat()ing file");
        }
        if (st.st_size == 0) {
            // Maybe the file doesn't support getting its size, double-check by
            // trying to tell() (seekable files usually have a size, while
            // non-seekable files don't)
            TURBO_RETURN_NOT_OK(file_tell(fd));
        } else if (st.st_size < 0) {
            return turbo::io_error("error getting file size");
        }
        return st.st_size;
    }

    TURBO_EXPORT bool file_is_closed(int fd) {
#if defined(_WIN32)
        // Disables default behavior on wrong params which causes the application to crash
      // https://msdn.microsoft.com/en-us/library/ksazx244.aspx
      _set_invalid_parameter_handler(InvalidParamHandler);

      // Disables possible assertion alert box on invalid input arguments
      _CrtSetReportMode(_CRT_ASSERT, 0);

      int new_fd = _dup(fd);
      if (new_fd == -1) {
        return errno == EBADF;
      }
      _close(new_fd);
      return false;
#else
        if (-1 != fcntl(fd, F_GETFD)) {
            return false;
        }
        return errno == EBADF;
#endif
    }

    turbo::Result<Pipe> create_pipe() {
        bool ok;
        int fds[2];
        Pipe pipe;

#if defined(_WIN32)
        ok = _pipe(fds, 4096, _O_BINARY) >= 0;
        if (ok) {
            pipe = {FileDescriptor(fds[0]), FileDescriptor(fds[1])};
        }
#elif defined(__linux__) && defined(__GLIBC__)
        // On Unix, we don't want the file descriptors to survive after an exec() call
        ok = pipe2(fds, O_CLOEXEC) >= 0;
        if (ok) {
            pipe = {FileDescriptor(fds[0]), FileDescriptor(fds[1])};
        }
#else
        auto set_cloexec = [](int fd) -> bool {
                int flags = fcntl(fd, F_GETFD);
                if (flags >= 0) {
                    flags = fcntl(fd, F_SETFD, flags | FD_CLOEXEC);
                }
                 return flags >= 0;
        };

        ok = ::pipe(fds) >= 0;
        if (ok) {
            pipe = {FileDescriptor(fds[0]), FileDescriptor(fds[1])};
            ok &= set_cloexec(fds[0]);
            if (ok) {
                ok &= set_cloexec(fds[1]);
            }
        }
#endif
        if (!ok) {
            return io_error_with_errno_payload(errno, "Error creating pipe");
        }

        return pipe;
    }

    turbo::Status set_pipe_non_blocking(int fd) {
#if defined(_WIN32)
        const auto handle = reinterpret_cast<HANDLE>(_get_osfhandle(fd));
        DWORD mode = PIPE_NOWAIT;
        if (!SetNamedPipeHandleState(handle, &mode, nullptr, nullptr)) {
            return io_error_with_errno_payload(GetLastError(), "Error making pipe non-blocking");
        }
#else
        int flags = fcntl(fd, F_GETFL);
        if (flags == -1 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
            return io_error_with_errno_payload(errno, "Error making pipe non-blocking");
        }
#endif
        return turbo::OkStatus();
    }


    namespace {

#ifdef WIN32
#define PIPE_WRITE _write
#define PIPE_READ _read
#else
#define PIPE_WRITE write
#define PIPE_READ read
#endif

        class SelfPipeImpl : public SelfPipe, public std::enable_shared_from_this<SelfPipeImpl> {
            static constexpr uint64_t kEofPayload = 5804561806345822987ULL;

        public:
            explicit SelfPipeImpl(bool signal_safe) : signal_safe_(signal_safe) {}

            turbo::Status init() {
                TURBO_MOVE_OR_RAISE(pipe_, create_pipe());
                if (signal_safe_) {
                    if (!please_shutdown_.is_lock_free()) {
                        return turbo::io_error("Cannot use non-lock-free atomic in a signal handler");
                    }
                    // We cannot afford blocking writes in a signal handler
                    TURBO_RETURN_NOT_OK(set_pipe_non_blocking(pipe_.wfd.fd()));
                }

                atfork_handler_ = std::make_shared<turbo::AtForkHandler>(
                        /*before=*/
                        [weak_self = std::weak_ptr<SelfPipeImpl>(shared_from_this())] {
                            auto self = weak_self.lock();
                            if (self) {
                                self->BeforeFork();
                            }
                            return self;
                        },
                        /*parent_after=*/
                        [](std::any token) {
                            auto self = std::any_cast<std::shared_ptr<SelfPipeImpl>>(std::move(token));
                            self->ParentAfterFork();
                        },
                        /*child_after=*/
                        [](std::any token) {
                            auto self = std::any_cast<std::shared_ptr<SelfPipeImpl>>(std::move(token));
                            self->ChildAfterFork();
                        });
                turbo::register_at_fork(atfork_handler_);

                return turbo::OkStatus();
            }

            turbo::Result<uint64_t> wait() override {
                if (pipe_.rfd.closed()) {
                    // Already closed
                    return closed_pipe();
                }
                uint64_t payload = 0;
                char *buf = reinterpret_cast<char *>(&payload);
                auto buf_size = static_cast<int64_t>(sizeof(payload));
                while (buf_size > 0) {
                    int64_t n_read = PIPE_READ(pipe_.rfd.fd(), buf, static_cast<uint32_t>(buf_size));
                    if (n_read < 0) {
                        if (errno == EINTR) {
                            continue;
                        }
                        if (pipe_.rfd.closed()) {
                            return closed_pipe();
                        }
                        return io_error_with_errno_payload(errno, "Failed reading from self-pipe");
                    }
                    buf += n_read;
                    buf_size -= n_read;
                }
                if (payload == kEofPayload && please_shutdown_.load()) {
                    TURBO_RETURN_NOT_OK(pipe_.rfd.close());
                    return closed_pipe();
                }
                return payload;
            }

            // XXX return StatusCode from here?
            void send(uint64_t payload) override {
                if (signal_safe_) {
                    int saved_errno = errno;
                    DoSend(payload);
                    errno = saved_errno;
                } else {
                    DoSend(payload);
                }
            }

            turbo::Status shutdown() override {
                please_shutdown_.store(true);
                errno = 0;
                if (!DoSend(kEofPayload)) {
                    if (errno) {
                        return io_error_with_errno_payload(errno, "Could not shutdown self-pipe");
                    } else if (!pipe_.wfd.closed()) {
                        return turbo::unknown_error("Could not shutdown self-pipe");
                    }
                }
                return pipe_.wfd.close();
            }

            ~SelfPipeImpl() { TURBO_WARN_NOT_OK(shutdown(), "On self-pipe destruction"); }

        protected:
            void BeforeFork() {}

            void ParentAfterFork() {}

            void ChildAfterFork() {
                // Close and recreate pipe, to avoid interfering with parent.
                const bool was_closed = pipe_.rfd.closed() || pipe_.wfd.closed();
                KCHECK_OK(pipe_.close());
                if (!was_closed) {
                    KCHECK_OK(create_pipe().try_value(&pipe_));
                }
            }

            turbo::Status closed_pipe() const { return turbo::invalid_argument_error("Self-pipe closed"); }

            bool DoSend(uint64_t payload) {
                // This needs to be async-signal safe as it's called from Send()
                if (pipe_.wfd.closed()) {
                    // Already closed
                    return false;
                }
                const char *buf = reinterpret_cast<const char *>(&payload);
                auto buf_size = static_cast<int64_t>(sizeof(payload));
                while (buf_size > 0) {
                    int64_t n_written =
                            PIPE_WRITE(pipe_.wfd.fd(), buf, static_cast<uint32_t>(buf_size));
                    if (n_written < 0) {
                        if (errno == EINTR) {
                            continue;
                        } else {
                            // Perhaps EAGAIN if non-blocking, or EBADF if closed in the meantime?
                            // In any case, we can't do anything more here.
                            break;
                        }
                    }
                    buf += n_written;
                    buf_size -= n_written;
                }
                return buf_size == 0;
            }

            const bool signal_safe_;
            Pipe pipe_;
            std::atomic<bool> please_shutdown_{false};

            std::shared_ptr<turbo::AtForkHandler> atfork_handler_;
        };

#undef PIPE_WRITE
#undef PIPE_READ

    }  // namespace

    turbo::Result<std::shared_ptr<SelfPipe>> SelfPipe::create(bool signal_safe) {
        auto ptr = std::make_shared<SelfPipeImpl>(signal_safe);
        TURBO_RETURN_NOT_OK(ptr->init());
        return ptr;
    }

    SelfPipe::~SelfPipe() = default;


    //
    // Compatible way to remap a memory map
    //

    turbo::Status StatusFromMmapErrno(const char *prefix) {
#ifdef _WIN32
        errno = __map_mman_error(GetLastError(), EPERM);
#endif
        return io_error_with_errno_payload(errno, prefix);
    }

    turbo::Status memory_map_remap(void *addr, size_t old_size, size_t new_size, int fildes,
                                   void **new_addr) {
        // should only be called with writable files
        *new_addr = MAP_FAILED;
#ifdef _WIN32
        // flags are ignored on windows
          HANDLE fm, h;

          if (!UnmapViewOfFile(addr)) {
            return StatusFromMmapErrno("UnmapViewOfFile failed");
          }

          h = reinterpret_cast<HANDLE>(_get_osfhandle(fildes));
          if (h == INVALID_HANDLE_VALUE) {
            return StatusFromMmapErrno("Cannot get file handle");
          }

          uint64_t new_size64 = new_size;
          LONG new_size_low = static_cast<LONG>(new_size64 & 0xFFFFFFFFUL);
          LONG new_size_high = static_cast<LONG>((new_size64 >> 32) & 0xFFFFFFFFUL);

          SetFilePointer(h, new_size_low, &new_size_high, FILE_BEGIN);
          SetEndOfFile(h);
          fm = CreateFileMapping(h, NULL, PAGE_READWRITE, 0, 0, "");
          if (fm == NULL) {
            return StatusFromMmapErrno("CreateFileMapping failed");
          }
          *new_addr = MapViewOfFile(fm, FILE_MAP_WRITE, 0, 0, new_size);
          CloseHandle(fm);
          if (new_addr == NULL) {
            return StatusFromMmapErrno("MapViewOfFile failed");
          }
          return turbo::OkStatus();
#elif defined(__linux__)
        if (ftruncate(fildes, new_size) == -1) {
            return StatusFromMmapErrno("ftruncate failed");
        }
        *new_addr = mremap(addr, old_size, new_size, MREMAP_MAYMOVE);
        if (*new_addr == MAP_FAILED) {
            return StatusFromMmapErrno("mremap failed");
        }
        return turbo::OkStatus();
#else
        // we have to close the mmap first, truncate the file to the new size
          // and recreate the mmap
          if (munmap(addr, old_size) == -1) {
            return StatusFromMmapErrno("munmap failed");
          }
          if (ftruncate(fildes, new_size) == -1) {
            return StatusFromMmapErrno("ftruncate failed");
          }
          // we set READ / WRITE flags on the new map, since we could only have
          // enlarged a RW map in the first place
          *new_addr = mmap(NULL, new_size, PROT_READ | PROT_WRITE, MAP_SHARED, fildes, 0);
          if (*new_addr == MAP_FAILED) {
            return StatusFromMmapErrno("mmap failed");
          }
          return turbo::OkStatus();
#endif
    }

    int64_t get_page_size_internal() {
#if defined(__APPLE__)
        return getpagesize();
#elif defined(_WIN32)
        SYSTEM_INFO si;
      GetSystemInfo(&si);
      return si.dwPageSize;
#else
        errno = 0;
        const auto ret = sysconf(_SC_PAGESIZE);
        if (ret == -1) {
            KLOG(FATAL) << "sysconf(_SC_PAGESIZE) failed: " << km_error(errno);
        }
        return static_cast<int64_t>(ret);
#endif
    }

    int64_t get_page_size() {
        static const int64_t kPageSize = get_page_size_internal();  // cache it
        return kPageSize;
    }


    // Returns the current resident set size (physical memory use) measured
    // in bytes, or zero if the value cannot be determined on this OS.
    int64_t get_current_rss() {
#if defined(_WIN32)
        // Windows --------------------------------------------------
          PROCESS_MEMORY_COUNTERS info;
          GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info));
          return static_cast<int64_t>(info.WorkingSetSize);

#elif defined(__APPLE__)
        // OSX ------------------------------------------------------
        #ifdef MACH_TASK_BASIC_INFO
              struct mach_task_basic_info info;
              mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;
              if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &infoCount) !=
                  KERN_SUCCESS) {
                  KLOG(WARNING) << "Can't resolve RSS value";
                  return 0;
              }
        #else
              struct task_basic_info info;
              mach_msg_type_number_t infoCount = TASK_BASIC_INFO_COUNT;
              if (task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&info, &infoCount) !=
                  KERN_SUCCESS) {
                  KLOG(WARNING) << "Can't resolve RSS value";
                  return 0;
              }
        #endif
          return static_cast<int64_t>(info.resident_size);

#elif defined(__linux__)
        // Linux ----------------------------------------------------
        int64_t rss = 0L;

        std::ifstream fp("/proc/self/statm");
        if (fp) {
            fp >> rss;
            return rss * sysconf(_SC_PAGESIZE);
        } else {
            KLOG(WARNING) << "Can't resolve RSS value from /proc/self/statm";
            return 0;
        }

#else
        // AIX, BSD, Solaris, and Unknown OS ------------------------
  return 0;  // Unsupported.
#endif
    }


    int64_t get_total_memory_bytes() {
#if defined(_WIN32)
        ULONGLONG result_kb;
  if (!GetPhysicallyInstalledSystemMemory(&result_kb)) {
      KLOG(WARNING) << "Failed to resolve total RAM size: "
                       << std::strerror(GetLastError());
    return -1;
  }
  return static_cast<int64_t>(result_kb * 1024);
#elif defined(__APPLE__)
        int64_t result;
  size_t size = sizeof(result);
  if (sysctlbyname("hw.memsize", &result, &size, nullptr, 0) == -1) {
    KLOG(WARNING) << "Failed to resolve total RAM size";
    return -1;
  }
  return result;
#elif defined(__linux__)
        struct sysinfo info;
        if (sysinfo(&info) == -1) {
            KLOG(WARNING) << "Failed to resolve total RAM size: " << std::strerror(errno);
            return -1;
        }
        return static_cast<int64_t>(info.totalram * info.mem_unit);
#else
        return 0;
#endif
    }

    turbo::Status memory_advise_will_need(const std::vector<MemoryRegion> &regions) {
#ifndef __EMSCRIPTEN__
        const auto page_size = static_cast<size_t>(get_page_size());
        DKCHECK_GT(page_size, 0);
        const size_t page_mask = ~(page_size - 1);
        DKCHECK_EQ(page_mask & page_size, page_size);

        auto align_region = [=](const MemoryRegion &region) -> MemoryRegion {
            const auto addr = reinterpret_cast<uintptr_t>(region.addr);
            const auto aligned_addr = addr & page_mask;
            DKCHECK_LT(addr - aligned_addr, page_size);
            return {reinterpret_cast<void *>(aligned_addr),
                    region.size + static_cast<size_t>(addr - aligned_addr)};
        };

#ifdef _WIN32
        // PrefetchVirtualMemory() is available on Windows 8 or later
          struct PrefetchEntry {  // Like WIN32_MEMORY_RANGE_ENTRY
            void* VirtualAddress;
            size_t NumberOfBytes;

            PrefetchEntry(const MemoryRegion& region)  // NOLINT runtime/explicit
                : VirtualAddress(region.addr), NumberOfBytes(region.size) {}
          };
          using PrefetchVirtualMemoryFunc = BOOL (*)(HANDLE, ULONG_PTR, PrefetchEntry*, ULONG);
          static const auto prefetch_virtual_memory = reinterpret_cast<PrefetchVirtualMemoryFunc>(
              GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "PrefetchVirtualMemory"));
          if (prefetch_virtual_memory != nullptr) {
            std::vector<PrefetchEntry> entries;
            entries.reserve(regions.size());
            for (const auto& region : regions) {
              if (region.size != 0) {
                entries.emplace_back(align_region(region));
              }
            }
            if (!entries.empty() &&
                !prefetch_virtual_memory(GetCurrentProcess(),
                                         static_cast<ULONG_PTR>(entries.size()), entries.data(),
                                         0)) {
              return io_error_with_errno_payload(GetLastError(), "PrefetchVirtualMemory failed");
            }
          }
          return turbo::OkStatus();
#elif defined(POSIX_MADV_WILLNEED)
        for (const auto &region: regions) {
            if (region.size != 0) {
                const auto aligned = align_region(region);
                int err = posix_madvise(aligned.addr, aligned.size, POSIX_MADV_WILLNEED);
                // EBADF can be returned on Linux in the following cases:
                // - the kernel version is older than 3.9
                // - the kernel was compiled with CONFIG_SWAP disabled (ARROW-9577)
                if (err != 0 && err != EBADF) {
                    return io_error_with_errno_payload(err, "posix_madvise failed");
                }
            }
        }
        return turbo::OkStatus();
#else
        return turbo::OkStatus();
#endif
#else
        return turbo::OkStatus();
#endif
    }

}  // namespace turbo
