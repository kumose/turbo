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
#include <turbo/files/file.h>
#include <turbo/files/filesystem.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <turbo/log/logging.h>
#include <turbo/base/internal/eintr_wrapper.h>
#include <turbo/threading/thread_restrictions.h>

namespace turbo {

    // Make sure our Whence mappings match the system headers.
    static_assert(File::FROM_BEGIN == SEEK_SET &&
                  File::FROM_CURRENT == SEEK_CUR &&
                  File::FROM_END == SEEK_END, "whence_matches_system");

    namespace {

#if defined(OS_BSD) || defined(OS_MACOSX) || defined(OS_NACL)
        static int CallFstat(int fd, stat_wrapper_t *sb) {
          turbo::ThreadRestrictions::AssertIOAllowed();
          return fstat(fd, sb);
        }
#else

        static int CallFstat(int fd, stat_wrapper_t *sb) {
            turbo::ThreadRestrictions::AssertIOAllowed();
            return fstat64(fd, sb);
        }

#endif

// NaCl doesn't provide the following system calls, so either simulate them or
// wrap them in order to minimize the number of #ifdef's in this file.
#if !defined(OS_NACL)

        static bool IsOpenAppend(PlatformFile file) {
            return (fcntl(file, F_GETFL) & O_APPEND) != 0;
        }

        static int CallFtruncate(PlatformFile file, int64_t length) {
            return HANDLE_EINTR(ftruncate(file, length));
        }

        static int CallFsync(PlatformFile file) {
            return HANDLE_EINTR(fsync(file));
        }

        static int CallFutimes(PlatformFile file, const struct timeval times[2]) {
#ifdef __USE_XOPEN2K8
            // futimens should be available, but futimes might not be
            // http://pubs.opengroup.org/onlinepubs/9699919799/

            timespec ts_times[2];
            ts_times[0].tv_sec = times[0].tv_sec;
            ts_times[0].tv_nsec = times[0].tv_usec * 1000;
            ts_times[1].tv_sec = times[1].tv_sec;
            ts_times[1].tv_nsec = times[1].tv_usec * 1000;

            return futimens(file, ts_times);
#else
            return futimes(file, times);
#endif
        }

        static File::Error CallFctnlFlock(PlatformFile file, bool do_lock) {
            struct flock lock;
            lock.l_type = F_WRLCK;
            lock.l_whence = SEEK_SET;
            lock.l_start = 0;
            lock.l_len = 0;  // Lock entire file.
            if (HANDLE_EINTR(fcntl(file, do_lock ? F_SETLK : F_UNLCK, &lock)) == -1)
                return File::OSErrorToFileError(errno);
            return File::FILE_OK;
        }

#else  // defined(OS_NACL)

        static bool IsOpenAppend(PlatformFile file) {
          // NaCl doesn't implement fcntl. Since NaCl's write conforms to the POSIX
          // standard and always appends if the file is opened with O_APPEND, just
          // return false here.
          return false;
        }

        static int CallFtruncate(PlatformFile file, int64_t length) {
          NOTIMPLEMENTED();  // NaCl doesn't implement ftruncate.
          return 0;
        }

        static int CallFsync(PlatformFile file) {
          NOTIMPLEMENTED();  // NaCl doesn't implement fsync.
          return 0;
        }

        static int CallFutimes(PlatformFile file, const struct timeval times[2]) {
          NOTIMPLEMENTED();  // NaCl doesn't implement futimes.
          return 0;
        }

        static File::Error CallFctnlFlock(PlatformFile file, bool do_lock) {
          NOTIMPLEMENTED();  // NaCl doesn't implement flock struct.
          return File::FILE_ERROR_INVALID_OPERATION;
        }
#endif  // defined(OS_NACL)

    }  // namespace

    void File::Info::from_stat(const stat_wrapper_t &stat_info) {
        is_directory = S_ISDIR(stat_info.st_mode);
        is_symbolic_link = S_ISLNK(stat_info.st_mode);
        size = stat_info.st_size;

#if defined(OS_LINUX)
        time_t last_modified_sec = stat_info.st_mtim.tv_sec;
        int64_t last_modified_nsec = stat_info.st_mtim.tv_nsec;
        time_t last_accessed_sec = stat_info.st_atim.tv_sec;
        int64_t last_accessed_nsec = stat_info.st_atim.tv_nsec;
        time_t creation_time_sec = stat_info.st_ctim.tv_sec;
        int64_t creation_time_nsec = stat_info.st_ctim.tv_nsec;
#elif defined(OS_ANDROID)
        time_t last_modified_sec = stat_info.st_mtime;
        int64_t last_modified_nsec = stat_info.st_mtime_nsec;
        time_t last_accessed_sec = stat_info.st_atime;
        int64_t last_accessed_nsec = stat_info.st_atime_nsec;
        time_t creation_time_sec = stat_info.st_ctime;
        int64_t creation_time_nsec = stat_info.st_ctime_nsec;
#elif defined(OS_MACOSX) || defined(OS_IOS) || defined(OS_BSD)
        time_t last_modified_sec = stat_info.st_mtimespec.tv_sec;
        int64_t last_modified_nsec = stat_info.st_mtimespec.tv_nsec;
        time_t last_accessed_sec = stat_info.st_atimespec.tv_sec;
        int64_t last_accessed_nsec = stat_info.st_atimespec.tv_nsec;
        time_t creation_time_sec = stat_info.st_ctimespec.tv_sec;
        int64_t creation_time_nsec = stat_info.st_ctimespec.tv_nsec;
#else
        time_t last_modified_sec = stat_info.st_mtime;
        int64_t last_modified_nsec = 0;
        time_t last_accessed_sec = stat_info.st_atime;
        int64_t last_accessed_nsec = 0;
        time_t creation_time_sec = stat_info.st_ctime;
        int64_t creation_time_nsec = 0;
#endif

        last_modified = turbo::Time::from_timespec({last_modified_sec, last_modified_nsec});

        last_accessed = turbo::Time::from_timespec({last_accessed_sec, last_accessed_nsec});

        creation_time = turbo::Time::from_timespec({creation_time_sec, creation_time_nsec});
    }

    // NaCl doesn't implement system calls to open files directly.
#if !defined(OS_NACL)

    // TODO(erikkay): does it make sense to support FLAG_EXCLUSIVE_* here?
    void File::initialize_unsafe(const turbo::FilePath &name, uint32_t flags) {
        turbo::ThreadRestrictions::AssertIOAllowed();
        DKCHECK(!is_valid());

        int open_flags = 0;
        if (flags & FLAG_CREATE)
            open_flags = O_CREAT | O_EXCL;

        created_ = false;

        if (flags & FLAG_CREATE_ALWAYS) {
            DKCHECK(!open_flags);
            DKCHECK(flags & FLAG_WRITE);
            open_flags = O_CREAT | O_TRUNC;
        }

        if (flags & FLAG_OPEN_TRUNCATED) {
            DKCHECK(!open_flags);
            DKCHECK(flags & FLAG_WRITE);
            open_flags = O_TRUNC;
        }

        if (!open_flags && !(flags & FLAG_OPEN) && !(flags & FLAG_OPEN_ALWAYS)) {
            KLOG(FATAL);
            errno = EOPNOTSUPP;
            error_details_ = FILE_ERROR_FAILED;
            return;
        }

        if (flags & FLAG_WRITE && flags & FLAG_READ) {
            open_flags |= O_RDWR;
        } else if (flags & FLAG_WRITE) {
            open_flags |= O_WRONLY;
        } else if (!(flags & FLAG_READ) &&
                   !(flags & FLAG_WRITE_ATTRIBUTES) &&
                   !(flags & FLAG_APPEND) &&
                   !(flags & FLAG_OPEN_ALWAYS)) {
            KLOG(FATAL);
        }

        if (flags & FLAG_TERMINAL_DEVICE)
            open_flags |= O_NOCTTY | O_NDELAY;

        if (flags & FLAG_APPEND && flags & FLAG_READ)
            open_flags |= O_APPEND | O_RDWR;
        else if (flags & FLAG_APPEND)
            open_flags |= O_APPEND | O_WRONLY;

        static_assert(O_RDONLY == 0, "O_RDONLY_must_equal_zero");

        int mode = S_IRUSR | S_IWUSR;
#if defined(OS_CHROMEOS)
        mode |= S_IRGRP | S_IROTH;
#endif

        int descriptor = HANDLE_EINTR(open(name.string().c_str(), open_flags, mode));

        if (flags & FLAG_OPEN_ALWAYS) {
            if (descriptor < 0) {
                open_flags |= O_CREAT;
                if (flags & FLAG_EXCLUSIVE_READ || flags & FLAG_EXCLUSIVE_WRITE)
                    open_flags |= O_EXCL;   // together with O_CREAT implies O_NOFOLLOW

                descriptor = HANDLE_EINTR(open(name.string().c_str(), open_flags, mode));
                if (descriptor >= 0)
                    created_ = true;
            }
        }

        if (descriptor < 0) {
            error_details_ = File::OSErrorToFileError(errno);
            return;
        }

        if (flags & (FLAG_CREATE_ALWAYS | FLAG_CREATE))
            created_ = true;

        if (flags & FLAG_DELETE_ON_CLOSE)
            unlink(name.string().c_str());

        async_ = ((flags & FLAG_ASYNC) == FLAG_ASYNC);
        error_details_ = FILE_OK;
        file_.reset(descriptor);
    }

#endif  // !defined(OS_NACL)

    bool File::is_valid() const {
        return file_.is_valid();
    }

    PlatformFile File::get_platform_file() const {
        return file_.get();
    }

    PlatformFile File::take_platform_file() {
        return file_.release();
    }

    void File::close() {
        if (!is_valid())
            return;

        turbo::ThreadRestrictions::AssertIOAllowed();
        file_.reset();
    }

    int64_t File::seek(Whence whence, int64_t offset) {
        turbo::ThreadRestrictions::AssertIOAllowed();
        DKCHECK(is_valid());

#if defined(OS_ANDROID)
        static_assert(sizeof(int64_t) == sizeof(off64_t), "off64_t_64_bit");
      return lseek64(file_.get(), static_cast<off64_t>(offset),
                     static_cast<int>(whence));
#else
        static_assert(sizeof(int64_t) == sizeof(off_t), "off_t_64_bit");
        return lseek(file_.get(), static_cast<off_t>(offset),
                     static_cast<int>(whence));
#endif
    }

    int File::read(int64_t offset, char *data, int size) {
        turbo::ThreadRestrictions::AssertIOAllowed();
        DKCHECK(is_valid());
        if (size < 0)
            return -1;

        int bytes_read = 0;
        int rv;
        do {
            rv = HANDLE_EINTR(pread(file_.get(), data + bytes_read,
                                    size - bytes_read, offset + bytes_read));
            if (rv <= 0)
                break;

            bytes_read += rv;
        } while (bytes_read < size);

        return bytes_read ? bytes_read : rv;
    }

    int File::read_at_current_pos(char *data, int size) {
        turbo::ThreadRestrictions::AssertIOAllowed();
        DKCHECK(is_valid());
        if (size < 0)
            return -1;

        int bytes_read = 0;
        int rv;
        do {
            rv = HANDLE_EINTR(::read(file_.get(), data + bytes_read, size - bytes_read));
            if (rv <= 0)
                break;

            bytes_read += rv;
        } while (bytes_read < size);

        return bytes_read ? bytes_read : rv;
    }

    int File::read_no_best_effort(int64_t offset, char *data, int size) {
        turbo::ThreadRestrictions::AssertIOAllowed();
        DKCHECK(is_valid());

        return HANDLE_EINTR(pread(file_.get(), data, size, offset));
    }

    int File::ReadAtCurrentPosNoBestEffort(char *data, int size) {
        turbo::ThreadRestrictions::AssertIOAllowed();
        DKCHECK(is_valid());
        if (size < 0)
            return -1;

        return HANDLE_EINTR(::read(file_.get(), data, size));
    }

    int File::write(int64_t offset, const char *data, int size) {
        turbo::ThreadRestrictions::AssertIOAllowed();

        if (IsOpenAppend(file_.get()))
            return write_at_current_pos(data, size);

        DKCHECK(is_valid());
        if (size < 0)
            return -1;

        int bytes_written = 0;
        int rv;
        do {
            rv = HANDLE_EINTR(pwrite(file_.get(), data + bytes_written,
                                     size - bytes_written, offset + bytes_written));
            if (rv <= 0)
                break;

            bytes_written += rv;
        } while (bytes_written < size);

        return bytes_written ? bytes_written : rv;
    }

    int File::write_at_current_pos(const char *data, int size) {
        turbo::ThreadRestrictions::AssertIOAllowed();
        DKCHECK(is_valid());
        if (size < 0) {
            return -1;
        }

        int bytes_written = 0;
        int rv;
        do {
            rv = HANDLE_EINTR(::write(file_.get(), data + bytes_written,
                                    size - bytes_written));
            if (rv <= 0)
                break;

            bytes_written += rv;
        } while (bytes_written < size);

        return bytes_written ? bytes_written : rv;
    }

    int File::write_at_current_pos_no_best_effort(const char *data, int size) {
        turbo::ThreadRestrictions::AssertIOAllowed();
        DKCHECK(is_valid());
        if (size < 0)
            return -1;

        return HANDLE_EINTR(::write(file_.get(), data, size));
    }

    int64_t File::get_length() {
        DKCHECK(is_valid());

        stat_wrapper_t file_info;
        if (CallFstat(file_.get(), &file_info))
            return false;

        return file_info.st_size;
    }

    bool File::set_length(int64_t length) {
        turbo::ThreadRestrictions::AssertIOAllowed();
        DKCHECK(is_valid());
        return !CallFtruncate(file_.get(), length);
    }

    bool File::flush() {
        turbo::ThreadRestrictions::AssertIOAllowed();
        DKCHECK(is_valid());
        return !CallFsync(file_.get());
    }

    bool File::set_times(turbo::Time last_access_time, turbo::Time last_modified_time) {
        turbo::ThreadRestrictions::AssertIOAllowed();
        DKCHECK(is_valid());

        timeval times[2];
        times[0] = turbo::Time::to_timeval(last_access_time);
        times[1] = turbo::Time::to_timeval(last_modified_time);

        return !CallFutimes(file_.get(), times);
    }

    bool File::get_info(Info *info) {
        DKCHECK(is_valid());

        stat_wrapper_t file_info;
        if (CallFstat(file_.get(), &file_info))
            return false;

        info->from_stat(file_info);
        return true;
    }

    File::Error File::lock() {
        return CallFctnlFlock(file_.get(), true);
    }

    File::Error File::unlock() {
        return CallFctnlFlock(file_.get(), false);
    }

    // Static.
    File::Error File::OSErrorToFileError(int saved_errno) {
        switch (saved_errno) {
            case EACCES:
            case EISDIR:
            case EROFS:
            case EPERM:
                return FILE_ERROR_ACCESS_DENIED;
#if !defined(OS_NACL)  // ETXTBSY not defined by NaCl.
            case ETXTBSY:
                return FILE_ERROR_IN_USE;
#endif
            case EEXIST:
                return FILE_ERROR_EXISTS;
            case ENOENT:
                return FILE_ERROR_NOT_FOUND;
            case EMFILE:
                return FILE_ERROR_TOO_MANY_OPENED;
            case ENOMEM:
                return FILE_ERROR_NO_MEMORY;
            case ENOSPC:
                return FILE_ERROR_NO_SPACE;
            case ENOTDIR:
                return FILE_ERROR_NOT_A_DIRECTORY;
            default:
                return FILE_ERROR_FAILED;
        }
    }

    void File::SetPlatformFile(PlatformFile file) {
        DKCHECK(!file_.is_valid());
        file_.reset(file);
    }

}  // namespace turbo
