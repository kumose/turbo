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

#include <turbo/files/filesystem.h>
#include <turbo/functional/function_ref.h>

namespace turbo {

    namespace detail {
        std::error_code make_error_code(portable_error err) {
#ifdef OS_WIN
            switch (err) {
                case portable_error::none:
                    return std::error_code();
                case portable_error::exists:
                    return std::error_code(ERROR_ALREADY_EXISTS, std::system_category());
                case portable_error::not_found:
                    return std::error_code(ERROR_PATH_NOT_FOUND, std::system_category());
                case portable_error::not_supported:
                    return std::error_code(ERROR_NOT_SUPPORTED, std::system_category());
                case portable_error::not_implemented:
                    return std::error_code(ERROR_CALL_NOT_IMPLEMENTED, std::system_category());
                case portable_error::invalid_argument:
                    return std::error_code(ERROR_INVALID_PARAMETER, std::system_category());
                case portable_error::is_a_directory:
#ifdef ERROR_DIRECTORY_NOT_SUPPORTED
                    return std::error_code(ERROR_DIRECTORY_NOT_SUPPORTED, std::system_category());
#else
                    return std::error_code(ERROR_NOT_SUPPORTED, std::system_category());
#endif
            }
#else
            switch (err) {
                case portable_error::none:
                    return std::error_code{};
                case portable_error::exists:
                    return std::error_code{EEXIST, std::system_category()};
                case portable_error::not_found:
                    return std::error_code{ENOENT, std::system_category()};
                case portable_error::not_supported:
                    return std::error_code{ENOTSUP, std::system_category()};
                case portable_error::not_implemented:
                    return std::error_code{ENOSYS, std::system_category()};
                case portable_error::invalid_argument:
                    return std::error_code{EINVAL, std::system_category()};
                case portable_error::is_a_directory:
                    return std::error_code{EISDIR, std::system_category()};
            }
#endif
            return std::error_code{};
        }

#ifdef OS_WIN
        std::error_code make_system_error(uint32_t err) {
            return std::error_code(err ? static_cast<int>(err) : static_cast<int>(::GetLastError()), std::system_category());
        }
#else

        std::error_code make_system_error(int err) {
            return std::error_code{err ? err : errno, std::system_category()};
        }

#endif

        template<typename strT, typename std::enable_if<FilePath::_is_basic_string<strT>::value, bool>::type = true>
        bool startsWith(const strT &what, const strT &with) {
            return with.length() <= what.length() && equal(with.begin(), with.end(), what.begin());
        }

        template<typename strT, typename std::enable_if<FilePath::_is_basic_string<strT>::value, bool>::type = true>
        bool endsWith(const strT &what, const strT &with) {
            return with.length() <= what.length() &&
                   what.compare(what.length() - with.length(), with.size(), with) == 0;
        }

        turbo::Status validate_path(std::string_view s) {
            if (s.find_first_of('\0') != std::string::npos) {
                return invalid_argument_error("Embedded NUL char in path: '", s, "'");
            }
            return turbo::OkStatus();
        }

    }  // namespace detail

    void FilePath::check_long_path() {
#if defined(OS_WIN)
        if (is_absolute() && _path.length() >= MAX_PATH - 12 && !detail::startsWith(_path, impl_string_type(GHC_PLATFORM_LITERAL("\\\\?\\")))) {
        postprocess_path_with_format(native_format);
    }
#endif
    }

    void FilePath::postprocess_path_with_format(FilePath::format fmt) {
        switch (fmt) {
#ifdef OS_WIN
            case FilePath::native_format:
            case FilePath::auto_format:
            case FilePath::generic_format:
            for (auto& c : _path) {
                if (c == generic_separator) {
                    c = preferred_separator;
                }
            }
            if (is_absolute() && _path.length() >= MAX_PATH - 12 && !detail::startsWith(_path, impl_string_type(GHC_PLATFORM_LITERAL("\\\\?\\")))) {
                _path = GHC_PLATFORM_LITERAL("\\\\?\\") + _path;
            }
            handle_prefixes();
            break;
#else
            case FilePath::auto_format:
            case FilePath::native_format:
            case FilePath::generic_format:
                // nothing to do
                break;
#endif
        }
        if (_path.length() > _prefixLength + 2 && _path[_prefixLength] == preferred_separator &&
            _path[_prefixLength + 1] == preferred_separator && _path[_prefixLength + 2] != preferred_separator) {
            impl_string_type::iterator new_end = std::unique(
                    _path.begin() + static_cast<string_type::difference_type>(_prefixLength) + 2, _path.end(),
                    [](FilePath::value_type lhs, FilePath::value_type rhs) {
                        return lhs == rhs && lhs == preferred_separator;
                    });
            _path.erase(new_end, _path.end());
        } else {
            impl_string_type::iterator new_end = std::unique(
                    _path.begin() + static_cast<string_type::difference_type>(_prefixLength), _path.end(),
                    [](FilePath::value_type lhs, FilePath::value_type rhs) {
                        return lhs == rhs && lhs == preferred_separator;
                    });
            _path.erase(new_end, _path.end());
        }
    }


    namespace detail {

        bool equals_simple_insensitive(const FilePath::value_type *str1, const FilePath::value_type *str2) {
#ifdef OS_WIN
#ifdef __GNUC__
            while (::tolower((unsigned char)*str1) == ::tolower((unsigned char)*str2++)) {
                if (*str1++ == 0)
                    return true;
            }
            return false;
#else  // __GNUC__
#ifdef GHC_USE_WCHAR_T
            return 0 == ::_wcsicmp(str1, str2);
#else   // GHC_USE_WCHAR_T
            return 0 == ::_stricmp(str1, str2);
#endif  // GHC_USE_WCHAR_T
#endif  // __GNUC__
#else   // OS_WIN
            return 0 == ::strcasecmp(str1, str2);
#endif  // OS_WIN
        }

        int compare_simple_insensitive(const FilePath::value_type *str1, size_t len1, const FilePath::value_type *str2,
                                       size_t len2) {
            while (len1 > 0 && len2 > 0 &&
                   ::tolower(static_cast<unsigned char>(*str1)) == ::tolower(static_cast<unsigned char>(*str2))) {
                --len1;
                --len2;
                ++str1;
                ++str2;
            }
            if (len1 && len2) {
                return *str1 < *str2 ? -1 : 1;
            }
            if (len1 == 0 && len2 == 0) {
                return 0;
            }
            return len1 == 0 ? -1 : 1;
        }

        const char *strerror_adapter(const char *gnu, char *) {
            return gnu;
        }

        const char *strerror_adapter(int posix, const char *buffer) {
            if (posix) {
                return "Error in strerror_r!";
            }
            return buffer;
        }

        template<typename ErrorNumber>
        std::string systemErrorText(ErrorNumber code = 0) {
#if defined(OS_WIN)
            LPVOID msgBuf;
        DWORD dw = code ? static_cast<DWORD>(code) : ::GetLastError();
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&msgBuf, 0, nullptr);
        std::string msg = toUtf8(std::wstring((LPWSTR)msgBuf));
        LocalFree(msgBuf);
        return msg;
#else
            char buffer[512];
            return strerror_adapter(strerror_r(code ? code : errno, buffer, sizeof(buffer)), buffer);
#endif
        }

#ifdef OS_WIN
        using CreateSymbolicLinkW_fp = BOOLEAN(WINAPI*)(LPCWSTR, LPCWSTR, DWORD);
    using CreateHardLinkW_fp = BOOLEAN(WINAPI*)(LPCWSTR, LPCWSTR, LPSECURITY_ATTRIBUTES);

    void create_symlink(const FilePath& target_name, const FilePath& new_symlink, bool to_directory, std::error_code& ec)
    {
        std::error_code tec;
        auto fs = status(target_name, tec);
        if ((fs.type() == FileType::directory && !to_directory) || (fs.type() == FileType::regular && to_directory)) {
            ec = detail::make_error_code(detail::portable_error::not_supported);
            return;
        }
#if defined(__GNUC__) && __GNUC__ >= 8
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#elif defined(_MSC_VER) && !defined(__INTEL_COMPILER) && !defined(__clang__)
#pragma warning(push)
#pragma warning(disable : 4191)
#endif
        static CreateSymbolicLinkW_fp api_call = reinterpret_cast<CreateSymbolicLinkW_fp>(GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "CreateSymbolicLinkW"));
#if defined(__GNUC__) && __GNUC__ >= 8
#pragma GCC diagnostic pop
#elif defined(_MSC_VER) && !defined(__INTEL_COMPILER) && !defined(__clang__)
#pragma warning(pop)
#endif
        if (api_call) {
            if (api_call(GHC_NATIVEWP(new_symlink), GHC_NATIVEWP(target_name), to_directory ? 1 : 0) == 0) {
                auto result = ::GetLastError();
                if (result == ERROR_PRIVILEGE_NOT_HELD && api_call(GHC_NATIVEWP(new_symlink), GHC_NATIVEWP(target_name), to_directory ? 3 : 2) != 0) {
                    return;
                }
                ec = detail::make_system_error(result);
            }
        }
        else {
            ec = detail::make_system_error(ERROR_NOT_SUPPORTED);
        }
    }

    void create_hardlink(const FilePath& target_name, const FilePath& new_hardlink, std::error_code& ec)
    {
#if defined(__GNUC__) && __GNUC__ >= 8
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#elif defined(_MSC_VER) && !defined(__INTEL_COMPILER) && !defined(__clang__)
#pragma warning(push)
#pragma warning(disable : 4191)
#endif
        static CreateHardLinkW_fp api_call = reinterpret_cast<CreateHardLinkW_fp>(GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "CreateHardLinkW"));
#if defined(__GNUC__) && __GNUC__ >= 8
#pragma GCC diagnostic pop
#elif defined(_MSC_VER) && !defined(__INTEL_COMPILER) && !defined(__clang__)
#pragma warning(pop)
#endif
        if (api_call) {
            if (api_call(GHC_NATIVEWP(new_hardlink), GHC_NATIVEWP(target_name), nullptr) == 0) {
                ec = detail::make_system_error();
            }
        }
        else {
            ec = detail::make_system_error(ERROR_NOT_SUPPORTED);
        }
    }

    FilePath getFullPathName(const wchar_t* p, std::error_code& ec)
    {
        ULONG size = ::GetFullPathNameW(p, 0, 0, 0);
        if (size) {
            std::vector<wchar_t> buf(size, 0);
            ULONG s2 = GetFullPathNameW(p, size, buf.data(), nullptr);
            if (s2 && s2 < size) {
                return FilePath(std::wstring(buf.data(), s2));
            }
        }
        ec = detail::make_system_error();
        return FilePath();
    }

#else

        void create_symlink(const FilePath &target_name, const FilePath &new_symlink, bool, std::error_code &ec) {
            if (::symlink(target_name.c_str(), new_symlink.c_str()) != 0) {
                ec = detail::make_system_error();
            }
        }

#ifndef OS_WEB

        void create_hardlink(const FilePath &target_name, const FilePath &new_hardlink, std::error_code &ec) {
            if (::link(target_name.c_str(), new_hardlink.c_str()) != 0) {
                ec = detail::make_system_error();
            }
        }

#endif
#endif

        template<typename T>
        FileStatus file_status_from_st_mode(T mode) {
#ifdef OS_WIN
            FileType ft = FileType::unknown;
            if ((mode & _S_IFDIR) == _S_IFDIR) {
                ft = FileType::directory;
            }
            else if ((mode & _S_IFREG) == _S_IFREG) {
                ft = FileType::regular;
            }
            else if ((mode & _S_IFCHR) == _S_IFCHR) {
                ft = FileType::character;
            }
            FilePerms prms = static_cast<FilePerms>(mode & 0xfff);
            return FileStatus(ft, prms);
#else
            FileType ft = FileType::unknown;
            if (S_ISDIR(mode)) {
                ft = FileType::directory;
            } else if (S_ISREG(mode)) {
                ft = FileType::regular;
            } else if (S_ISCHR(mode)) {
                ft = FileType::character;
            } else if (S_ISBLK(mode)) {
                ft = FileType::block;
            } else if (S_ISFIFO(mode)) {
                ft = FileType::fifo;
            } else if (S_ISLNK(mode)) {
                ft = FileType::symlink;
            } else if (S_ISSOCK(mode)) {
                ft = FileType::socket;
            }
            FilePerms prms = static_cast<FilePerms>(mode & 0xfff);
            return FileStatus(ft, prms);
#endif
        }

#ifdef OS_WIN

        class unique_handle
        {
        public:
            typedef HANDLE element_type;

            unique_handle() noexcept
                : _handle(INVALID_HANDLE_VALUE)
            {
            }
            explicit unique_handle(element_type h) noexcept
                : _handle(h)
            {
            }
            unique_handle(unique_handle&& u) noexcept
                : _handle(u.release())
            {
            }
            ~unique_handle() { reset(); }
            unique_handle& operator=(unique_handle&& u) noexcept
            {
                reset(u.release());
                return *this;
            }
            element_type get() const noexcept { return _handle; }
            explicit operator bool() const noexcept { return _handle != INVALID_HANDLE_VALUE; }
            element_type release() noexcept
            {
                element_type tmp = _handle;
                _handle = INVALID_HANDLE_VALUE;
                return tmp;
            }
            void reset(element_type h = INVALID_HANDLE_VALUE) noexcept
            {
                element_type tmp = _handle;
                _handle = h;
                if (tmp != INVALID_HANDLE_VALUE) {
                    CloseHandle(tmp);
                }
            }
            void swap(unique_handle& u) noexcept { std::swap(_handle, u._handle); }

        private:
            element_type _handle;
        };

#ifndef REPARSE_DATA_BUFFER_HEADER_SIZE
        typedef struct _REPARSE_DATA_BUFFER
        {
            ULONG ReparseTag;
            USHORT ReparseDataLength;
            USHORT Reserved;
            union
            {
                struct
                {
                    USHORT SubstituteNameOffset;
                    USHORT SubstituteNameLength;
                    USHORT PrintNameOffset;
                    USHORT PrintNameLength;
                    ULONG Flags;
                    WCHAR PathBuffer[1];
                } SymbolicLinkReparseBuffer;
                struct
                {
                    USHORT SubstituteNameOffset;
                    USHORT SubstituteNameLength;
                    USHORT PrintNameOffset;
                    USHORT PrintNameLength;
                    WCHAR PathBuffer[1];
                } MountPointReparseBuffer;
                struct
                {
                    UCHAR DataBuffer[1];
                } GenericReparseBuffer;
            } DUMMYUNIONNAME;
        } REPARSE_DATA_BUFFER;
#ifndef MAXIMUM_REPARSE_DATA_BUFFER_SIZE
#define MAXIMUM_REPARSE_DATA_BUFFER_SIZE (16 * 1024)
#endif
#endif

        template <class T>
        struct free_deleter
        {
            void operator()(T* p) const { std::free(p); }
        };

        std::unique_ptr<REPARSE_DATA_BUFFER, free_deleter<REPARSE_DATA_BUFFER>> getReparseData(const FilePath& p, std::error_code& ec)
        {
            unique_handle file(CreateFileW(GHC_NATIVEWP(p), 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, 0));
            if (!file) {
                ec = detail::make_system_error();
                return nullptr;
            }

            std::unique_ptr<REPARSE_DATA_BUFFER, free_deleter<REPARSE_DATA_BUFFER>> reparseData(reinterpret_cast<REPARSE_DATA_BUFFER*>(std::calloc(1, MAXIMUM_REPARSE_DATA_BUFFER_SIZE)));
            ULONG bufferUsed;
            if (DeviceIoControl(file.get(), FSCTL_GET_REPARSE_POINT, 0, 0, reparseData.get(), MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &bufferUsed, 0)) {
                return reparseData;
            }
            else {
                ec = detail::make_system_error();
            }
            return nullptr;
        }
#endif

        FilePath resolveSymlink(const FilePath &p, std::error_code &ec) {
#ifdef OS_WIN
            FilePath result;
            auto reparseData = detail::getReparseData(p, ec);
            if (!ec) {
                if (reparseData && IsReparseTagMicrosoft(reparseData->ReparseTag)) {
                    switch (reparseData->ReparseTag) {
                        case IO_REPARSE_TAG_SYMLINK: {
                            auto printName = std::wstring(&reparseData->SymbolicLinkReparseBuffer.PathBuffer[reparseData->SymbolicLinkReparseBuffer.PrintNameOffset / sizeof(WCHAR)], reparseData->SymbolicLinkReparseBuffer.PrintNameLength / sizeof(WCHAR));
                            auto substituteName =
                                std::wstring(&reparseData->SymbolicLinkReparseBuffer.PathBuffer[reparseData->SymbolicLinkReparseBuffer.SubstituteNameOffset / sizeof(WCHAR)], reparseData->SymbolicLinkReparseBuffer.SubstituteNameLength / sizeof(WCHAR));
                            if (detail::endsWith(substituteName, printName) && detail::startsWith(substituteName, std::wstring(L"\\??\\"))) {
                                result = printName;
                            }
                            else {
                                result = substituteName;
                            }
                            if (reparseData->SymbolicLinkReparseBuffer.Flags & 0x1 /*SYMLINK_FLAG_RELATIVE*/) {
                                result = p.parent_path() / result;
                            }
                            break;
                        }
                        case IO_REPARSE_TAG_MOUNT_POINT:
                            result = detail::getFullPathName(GHC_NATIVEWP(p), ec);
                            // result = std::wstring(&reparseData->MountPointReparseBuffer.PathBuffer[reparseData->MountPointReparseBuffer.SubstituteNameOffset / sizeof(WCHAR)], reparseData->MountPointReparseBuffer.SubstituteNameLength / sizeof(WCHAR));
                            break;
                        default:
                            break;
                    }
                }
            }
            return result;
#else
            size_t bufferSize = 256;
            while (true) {
                std::vector<char> buffer(bufferSize, static_cast<char>(0));
                auto rc = ::readlink(p.c_str(), buffer.data(), buffer.size());
                if (rc < 0) {
                    ec = detail::make_system_error();
                    return FilePath{};
                } else if (rc < static_cast<int>(bufferSize)) {
                    return FilePath{std::string(buffer.data(), static_cast<std::string::size_type>(rc))};
                }
                bufferSize *= 2;
            }
            return FilePath{};
#endif
        }

#ifdef OS_WIN
        time_t timeFromFILETIME(const FILETIME& ft)
        {
            ULARGE_INTEGER ull;
            ull.LowPart = ft.dwLowDateTime;
            ull.HighPart = ft.dwHighDateTime;
            return static_cast<time_t>(ull.QuadPart / 10000000ULL - 11644473600ULL);
        }

        void timeToFILETIME(time_t t, FILETIME& ft)
        {
            ULARGE_INTEGER ull;
            ull.QuadPart = static_cast<ULONGLONG>((t * 10000000LL) + 116444736000000000LL);
            ft.dwLowDateTime = ull.LowPart;
            ft.dwHighDateTime = ull.HighPart;
        }

        template <typename INFO>
        uintmax_t hard_links_from_INFO(const INFO* info)
        {
            return static_cast<uintmax_t>(-1);
        }

        template <>
        uintmax_t hard_links_from_INFO<BY_HANDLE_FILE_INFORMATION>(const BY_HANDLE_FILE_INFORMATION* info)
        {
            return info->nNumberOfLinks;
        }

        template <typename INFO>
        bool is_symlink_from_INFO(const FilePath &p, const INFO* info, std::error_code& ec)
        {
            if ((info->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
                auto reparseData = detail::getReparseData(p, ec);
                if (!ec && reparseData && IsReparseTagMicrosoft(reparseData->ReparseTag) && reparseData->ReparseTag == IO_REPARSE_TAG_SYMLINK) {
                    return true;
                }
            }
            return false;
        }

        template <>
        bool is_symlink_from_INFO(const FilePath &, const WIN32_FIND_DATAW* info, std::error_code&)
        {
            // dwReserved0 is undefined unless dwFileAttributes includes the
            // FILE_ATTRIBUTE_REPARSE_POINT attribute according to microsoft
            // documentation. In practice, dwReserved0 is not reset which
            // causes it to report the incorrect symlink status.
            // Note that microsoft documentation does not say whether there is
            // a null value for dwReserved0, so we test for symlink directly
            // instead of returning the tag which requires returning a null
            // value for non-reparse-point files.
            return (info->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) && info->dwReserved0 == IO_REPARSE_TAG_SYMLINK;
        }

        template <typename INFO>
        FileStatus status_from_INFO(const FilePath& p, const INFO* info, std::error_code& ec, uintmax_t* sz = nullptr, time_t* lwt = nullptr)
        {
            FileType ft = FileType::unknown;
            if (is_symlink_from_INFO(p, info, ec)) {
                ft = FileType::symlink;
            }
            else if ((info->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                ft = FileType::directory;
            }
            else {
                ft = FileType::regular;
            }
            FilePerms prms = FilePerms::owner_read | FilePerms::group_read | FilePerms::others_read;
            if (!(info->dwFileAttributes & FILE_ATTRIBUTE_READONLY)) {
                prms = prms | FilePerms::owner_write | FilePerms::group_write | FilePerms::others_write;
            }
            if (has_executable_extension(p)) {
                prms = prms | FilePerms::owner_exec | FilePerms::group_exec | FilePerms::others_exec;
            }
            if (sz) {
                *sz = static_cast<uintmax_t>(info->nFileSizeHigh) << (sizeof(info->nFileSizeHigh) * 8) | info->nFileSizeLow;
            }
            if (lwt) {
                *lwt = detail::timeFromFILETIME(info->ftLastWriteTime);
            }
            return FileStatus(ft, prms);
        }

#endif

        bool is_not_found_error(std::error_code &ec) {
#ifdef OS_WIN
            return ec.value() == ERROR_FILE_NOT_FOUND || ec.value() == ERROR_PATH_NOT_FOUND || ec.value() == ERROR_INVALID_NAME;
#else
            return ec.value() == ENOENT || ec.value() == ENOTDIR;
#endif
        }

        FileStatus
        symlink_status_ex(const FilePath &p, std::error_code &ec, uintmax_t *sz = nullptr, uintmax_t *nhl = nullptr,
                          time_t *lwt = nullptr) noexcept {
#ifdef OS_WIN
            FileStatus fs;
            WIN32_FILE_ATTRIBUTE_DATA attr;
            if (!GetFileAttributesExW(GHC_NATIVEWP(p), GetFileExInfoStandard, &attr)) {
                ec = detail::make_system_error();
            }
            else {
                ec.clear();
                fs = detail::status_from_INFO(p, &attr, ec, sz, lwt);
                if (nhl) {
                    *nhl = 0;
                }
            }
            if (detail::is_not_found_error(ec)) {
                return FileStatus(FileType::not_found);
            }
            return ec ? FileStatus(FileType::none) : fs;
#else
            (void) sz;
            (void) nhl;
            (void) lwt;
            struct ::stat fs;
            auto result = ::lstat(p.c_str(), &fs);
            if (result == 0) {
                ec.clear();
                FileStatus f_s = detail::file_status_from_st_mode(fs.st_mode);
                return f_s;
            }
            ec = detail::make_system_error();
            if (detail::is_not_found_error(ec)) {
                return FileStatus(FileType::not_found, FilePerms::unknown);
            }
            return FileStatus(FileType::none);
#endif
        }

        FileStatus status_ex(const FilePath &p, std::error_code &ec, FileStatus *sls = nullptr, uintmax_t *sz = nullptr,
                             uintmax_t *nhl = nullptr, time_t *lwt = nullptr, int recurse_count = 0) noexcept {
            ec.clear();
#ifdef OS_WIN
            if (recurse_count > 16) {
                ec = detail::make_system_error(0x2A9 /*ERROR_STOPPED_ON_SYMLINK*/);
                return FileStatus(FileType::unknown);
            }
            WIN32_FILE_ATTRIBUTE_DATA attr;
            if (!::GetFileAttributesExW(GHC_NATIVEWP(p), GetFileExInfoStandard, &attr)) {
                ec = detail::make_system_error();
            }
            else if (attr.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
                auto reparseData = detail::getReparseData(p, ec);
                if (!ec && reparseData && IsReparseTagMicrosoft(reparseData->ReparseTag) && reparseData->ReparseTag == IO_REPARSE_TAG_SYMLINK) {
                    FilePath target = resolveSymlink(p, ec);
                    FileStatus result;
                    if (!ec && !target.empty()) {
                        if (sls) {
                            *sls = status_from_INFO(p, &attr, ec);
                        }
                        return detail::status_ex(target, ec, nullptr, sz, nhl, lwt, recurse_count + 1);
                    }
                    return FileStatus(FileType::unknown);
                }
            }
            if (ec) {
                if (detail::is_not_found_error(ec)) {
                    return FileStatus(FileType::not_found);
                }
                return FileStatus(FileType::none);
            }
            if (nhl) {
                *nhl = 0;
            }
            return detail::status_from_INFO(p, &attr, ec, sz, lwt);
#else
            (void) recurse_count;
            struct ::stat st;
            auto result = ::lstat(p.c_str(), &st);
            if (result == 0) {
                ec.clear();
                FileStatus fs = detail::file_status_from_st_mode(st.st_mode);
                if (sls) {
                    *sls = fs;
                }
                if (fs.type() == FileType::symlink) {
                    result = ::stat(p.c_str(), &st);
                    if (result == 0) {
                        fs = detail::file_status_from_st_mode(st.st_mode);
                    } else {
                        ec = detail::make_system_error();
                        if (detail::is_not_found_error(ec)) {
                            return FileStatus(FileType::not_found, FilePerms::unknown);
                        }
                        return FileStatus(FileType::none);
                    }
                }
                if (sz) {
                    *sz = static_cast<uintmax_t>(st.st_size);
                }
                if (nhl) {
                    *nhl = st.st_nlink;
                }
                if (lwt) {
                    *lwt = st.st_mtime;
                }
                return fs;
            } else {
                ec = detail::make_system_error();
                if (detail::is_not_found_error(ec)) {
                    return FileStatus(FileType::not_found, FilePerms::unknown);
                }
                return FileStatus(FileType::none);
            }
#endif
        }

    }  // namespace detail
    u8arguments::u8arguments(int &argc, char **&argv)
            : _argc(argc), _argv(argv), _refargc(argc), _refargv(argv), _isvalid(false) {
#ifdef OS_WIN
        LPWSTR* p;
    p = ::CommandLineToArgvW(::GetCommandLineW(), &argc);
    _args.reserve(static_cast<size_t>(argc));
    _argp.reserve(static_cast<size_t>(argc));
    for (size_t i = 0; i < static_cast<size_t>(argc); ++i) {
        _args.push_back(detail::toUtf8(std::wstring(p[i])));
        _argp.push_back((char*)_args[i].data());
    }
    argv = _argp.data();
    ::LocalFree(p);
    _isvalid = true;
#else
        std::setlocale(LC_ALL, "");
#if defined(__ANDROID__) && __ANDROID_API__ < 26
        _isvalid = true;
#else
        if (detail::equals_simple_insensitive(::nl_langinfo(CODESET), "UTF-8")) {
            _isvalid = true;
        }
#endif
#endif
    }
    //-----------------------------------------------------------------------------
    // [fs.path.construct] constructors and destructor

    FilePath::FilePath() noexcept {}

    FilePath::FilePath(const FilePath &p)
            : _path(p._path)
#if defined(OS_WIN)
    , _prefixLength(p._prefixLength)
#endif
    {
    }

    FilePath::FilePath(FilePath &&p) noexcept
            : _path(std::move(p._path))
#if defined(OS_WIN)
    , _prefixLength(p._prefixLength)
#endif
    {
    }

    FilePath::FilePath(string_type &&source, format fmt)
            : _path(std::move(source)) {
        postprocess_path_with_format(fmt);
    }

    //-----------------------------------------------------------------------------
    // [fs.path.assign] assignments

    FilePath &FilePath::operator=(const FilePath &p) {
        _path = p._path;
#if defined(OS_WIN)
        _prefixLength = p._prefixLength;
#endif
        return *this;
    }

    FilePath &FilePath::operator=(FilePath &&p) noexcept {
        _path = std::move(p._path);
#if defined(OS_WIN)
        _prefixLength = p._prefixLength;
#endif
        return *this;
    }

    FilePath &FilePath::operator=(FilePath::string_type &&source) {
        return assign(source);
    }

    FilePath &FilePath::assign(FilePath::string_type &&source) {
        _path = std::move(source);
        postprocess_path_with_format(native_format);
        return *this;
    }
    //-----------------------------------------------------------------------------
    // [fs.path.append] appends

    FilePath &FilePath::operator/=(const FilePath &p) {
        if (p.empty()) {
            // was: if ((!has_root_directory() && is_absolute()) || has_filename())
            if (!_path.empty() && _path[_path.length() - 1] != preferred_separator &&
                _path[_path.length() - 1] != ':') {
                _path += preferred_separator;
            }
            return *this;
        }
        if ((p.is_absolute() && (_path != root_name()._path || p._path != "/")) ||
            (p.has_root_name() && p.root_name() != root_name())) {
            assign(p);
            return *this;
        }
        if (p.has_root_directory()) {
            assign(root_name());
        } else if ((!has_root_directory() && is_absolute()) || has_filename()) {
            _path += preferred_separator;
        }
        auto iter = p.begin();
        bool first = true;
        if (p.has_root_name()) {
            ++iter;
        }
        while (iter != p.end()) {
            if (!first && !(!_path.empty() && _path[_path.length() - 1] == preferred_separator)) {
                _path += preferred_separator;
            }
            first = false;
            _path += (*iter++).native();
        }
        check_long_path();
        return *this;
    }

    void FilePath::append_name(const value_type *name) {
        if (_path.empty()) {
            this->operator/=(FilePath(name));
        } else {
            if (_path.back() != FilePath::preferred_separator) {
                _path.push_back(FilePath::preferred_separator);
            }
            _path += name;
            check_long_path();
        }
    }


    //-----------------------------------------------------------------------------
    // [fs.path.concat] concatenation

    FilePath &FilePath::operator+=(const FilePath &x) {
        return concat(x._path);
    }

    FilePath &FilePath::operator+=(const string_type &x) {
        return concat(x);
    }

    FilePath &FilePath::operator+=(basic_string_view<value_type> x) {
        return concat(x);
    }

    FilePath &FilePath::operator+=(const value_type *x) {
        basic_string_view<value_type> part(x);
        return concat(part);
    }

    FilePath &FilePath::operator+=(value_type x) {
#ifdef OS_WIN
        if (x == generic_separator) {
        x = preferred_separator;
    }
#endif
        if (_path.empty() || _path.back() != preferred_separator) {
            _path += x;
        }
        check_long_path();
        return *this;
    }

    //-----------------------------------------------------------------------------
    // [fs.path.modifiers] modifiers
    void FilePath::clear() noexcept {
        _path.clear();
#if defined(OS_WIN)
        _prefixLength = 0;
#endif
    }

    FilePath &FilePath::make_preferred() {
        // as this filesystem implementation only uses generic_format
        // internally, this must be a no-op
        return *this;
    }

    FilePath &FilePath::remove_filename() {
        if (has_filename()) {
            _path.erase(_path.size() - filename()._path.size());
        }
        return *this;
    }

    FilePath &FilePath::replace_filename(const FilePath &replacement) {
        remove_filename();
        return append(replacement);
    }

    FilePath &FilePath::replace_extension(const FilePath &replacement) {
        if (has_extension()) {
            _path.erase(_path.size() - extension()._path.size());
        }
        if (!replacement.empty() && replacement._path[0] != '.') {
            _path += '.';
        }
        return concat(replacement);
    }

    void FilePath::swap(FilePath &rhs) noexcept {
        _path.swap(rhs._path);
#if defined(OS_WIN)
        std::swap(_prefixLength, rhs._prefixLength);
#endif
    }

    //-----------------------------------------------------------------------------
    // [fs.path.native.obs] native format observers
    const FilePath::string_type &FilePath::native() const noexcept {
        return _path;
    }

    const FilePath::value_type *FilePath::c_str() const noexcept {
        return native().c_str();
    }

    FilePath::operator FilePath::string_type() const {
        return native();
    }


    std::string FilePath::string() const {
#ifdef GHC_USE_WCHAR_T
        return detail::toUtf8(native());
#else
        return native();
#endif
    }

    std::wstring FilePath::wstring() const {
#ifdef GHC_USE_WCHAR_T
        return native();
#else
        return detail::fromUtf8<std::wstring>(native());
#endif
    }

#if defined(__cpp_lib_char8_t)
    std::u8string FilePath::u8string() const
{
#ifdef GHC_USE_WCHAR_T
    return std::u8string(reinterpret_cast<const char8_t*>(detail::toUtf8(native()).c_str()));
#else
    return std::u8string(reinterpret_cast<const char8_t*>(c_str()));
#endif
}
#else

    std::string FilePath::u8string() const {
#ifdef GHC_USE_WCHAR_T
        return detail::toUtf8(native());
#else
        return native();
#endif
    }

#endif

    std::u16string FilePath::u16string() const {
        // TODO: optimize
        return detail::fromUtf8<std::u16string>(string());
    }

    std::u32string FilePath::u32string() const {
        // TODO: optimize
        return detail::fromUtf8<std::u32string>(string());
    }

    std::string FilePath::generic_string() const {
#ifdef OS_WIN
        return generic_string<std::string::value_type, std::string::traits_type, std::string::allocator_type>();
#else
        return _path;
#endif
    }

    std::wstring FilePath::generic_wstring() const {
#ifdef OS_WIN
        return generic_string<std::wstring::value_type, std::wstring::traits_type, std::wstring::allocator_type>();
#else
        return detail::fromUtf8<std::wstring>(_path);
#endif
    }  // namespace filesystem

#if defined(__cpp_lib_char8_t)
    std::u8string FilePath::generic_u8string() const
{
#ifdef OS_WIN
    return generic_string<std::u8string::value_type, std::u8string::traits_type, std::u8string::allocator_type>();
#else
    return std::u8string(reinterpret_cast<const char8_t*>(_path.c_str()));
#endif
}
#else

    std::string FilePath::generic_u8string() const {
#ifdef OS_WIN
        return generic_string<std::string::value_type, std::string::traits_type, std::string::allocator_type>();
#else
        return _path;
#endif
    }

#endif

    std::u16string FilePath::generic_u16string() const {
#ifdef OS_WIN
        return generic_string<std::u16string::value_type, std::u16string::traits_type, std::u16string::allocator_type>();
#else
        return detail::fromUtf8<std::u16string>(_path);
#endif
    }

    std::u32string FilePath::generic_u32string() const {
#ifdef OS_WIN
        return generic_string<std::u32string::value_type, std::u32string::traits_type, std::u32string::allocator_type>();
#else
        return detail::fromUtf8<std::u32string>(_path);
#endif
    }


    //-----------------------------------------------------------------------------
    // [fs.path.compare] compare
    int FilePath::compare(const FilePath &p) const noexcept {
        auto rnl1 = root_name_length();
        auto rnl2 = p.root_name_length();
#ifdef OS_WIN
        auto rnc = detail::compare_simple_insensitive(_path.c_str(), rnl1, p._path.c_str(), rnl2);
#else
        auto rnc = _path.compare(0, rnl1, p._path, 0, (std::min(rnl1, rnl2)));
#endif
        if (rnc) {
            return rnc;
        }
        bool hrd1 = has_root_directory(), hrd2 = p.has_root_directory();
        if (hrd1 != hrd2) {
            return hrd1 ? 1 : -1;
        }
        if (hrd1) {
            ++rnl1;
            ++rnl2;
        }
        auto iter1 = _path.begin() + static_cast<int>(rnl1);
        auto iter2 = p._path.begin() + static_cast<int>(rnl2);
        while (iter1 != _path.end() && iter2 != p._path.end() && *iter1 == *iter2) {
            ++iter1;
            ++iter2;
        }
        if (iter1 == _path.end()) {
            return iter2 == p._path.end() ? 0 : -1;
        }
        if (iter2 == p._path.end()) {
            return 1;
        }
        if (*iter1 == preferred_separator) {
            return -1;
        }
        if (*iter2 == preferred_separator) {
            return 1;
        }
        return *iter1 < *iter2 ? -1 : 1;
    }

    int FilePath::compare(const string_type &s) const {
        return compare(FilePath(s));
    }

    int FilePath::compare(basic_string_view<value_type> s) const {
        return compare(FilePath(s));
    }

    int FilePath::compare(const value_type *s) const {
        return compare(FilePath(s));
    }

    //-----------------------------------------------------------------------------
    // [fs.path.decompose] decomposition
#ifdef OS_WIN
    void FilePath::handle_prefixes()
{
    _prefixLength = 0;
    if (_path.length() >= 6 && _path[2] == '?' && std::toupper(static_cast<unsigned char>(_path[4])) >= 'A' && std::toupper(static_cast<unsigned char>(_path[4])) <= 'Z' && _path[5] == ':') {
        if (detail::startsWith(_path, impl_string_type(GHC_PLATFORM_LITERAL("\\\\?\\"))) || detail::startsWith(_path, impl_string_type(GHC_PLATFORM_LITERAL("\\??\\")))) {
            _prefixLength = 4;
        }
    }
}
#endif

    FilePath::string_type::size_type FilePath::root_name_length() const noexcept {
#ifdef OS_WIN
        if (_path.length() >= _prefixLength + 2 && std::toupper(static_cast<unsigned char>(_path[_prefixLength])) >= 'A' && std::toupper(static_cast<unsigned char>(_path[_prefixLength])) <= 'Z' && _path[_prefixLength + 1] == ':') {
        return 2;
    }
#endif
        if (_path.length() > _prefixLength + 2 && _path[_prefixLength] == preferred_separator &&
            _path[_prefixLength + 1] == preferred_separator && _path[_prefixLength + 2] != preferred_separator &&
            std::isprint(_path[_prefixLength + 2])) {
            impl_string_type::size_type pos = _path.find(preferred_separator, _prefixLength + 3);
            if (pos == impl_string_type::npos) {
                return _path.length();
            } else {
                return pos;
            }
        }
        return 0;
    }

    FilePath FilePath::root_name() const {
        return FilePath(_path.substr(_prefixLength, root_name_length()), native_format);
    }

    FilePath FilePath::root_directory() const {
        if (has_root_directory()) {
            static const FilePath _root_dir(std::string(1, preferred_separator), native_format);
            return _root_dir;
        }
        return FilePath();
    }

    FilePath FilePath::root_path() const {
        return FilePath(root_name().string() + root_directory().string(), native_format);
    }

    FilePath FilePath::relative_path() const {
        auto rootPathLen = _prefixLength + root_name_length() + (has_root_directory() ? 1 : 0);
        return FilePath(_path.substr((std::min)(rootPathLen, _path.length())), generic_format);
    }

    FilePath FilePath::parent_path() const {
        auto rootPathLen = _prefixLength + root_name_length() + (has_root_directory() ? 1 : 0);
        if (rootPathLen < _path.length()) {
            if (empty()) {
                return FilePath();
            } else {
                auto piter = end();
                auto iter = piter.decrement(_path.end());
                if (iter > _path.begin() + static_cast<long>(rootPathLen) && *iter != preferred_separator) {
                    --iter;
                }
                return FilePath(_path.begin(), iter, native_format);
            }
        } else {
            return *this;
        }
    }

    FilePath FilePath::filename() const {
        return !has_relative_path() ? FilePath() : FilePath(*--end());
    }

    FilePath FilePath::stem() const {
        impl_string_type fn = filename().native();
        if (fn != "." && fn != "..") {
            impl_string_type::size_type pos = fn.rfind('.');
            if (pos != impl_string_type::npos && pos > 0) {
                return FilePath{fn.substr(0, pos), native_format};
            }
        }
        return FilePath{fn, native_format};
    }

    FilePath FilePath::extension() const {
        if (has_relative_path()) {
            auto iter = end();
            const auto &fn = *--iter;
            impl_string_type::size_type pos = fn._path.rfind('.');
            if (pos != std::string::npos && pos > 0 && fn._path != "..") {
                return FilePath(fn._path.substr(pos), native_format);
            }
        }
        return FilePath();
    }

#ifdef OS_WIN
    namespace detail {
        bool has_executable_extension(const FilePath& p)
        {
            if (p.has_relative_path()) {
                auto iter = p.end();
                const auto& fn = *--iter;
                auto pos = fn._path.find_last_of('.');
                if (pos == std::string::npos || pos == 0 || fn._path.length() - pos != 3) {
                    return false;
                }
                const FilePath::value_type* ext = fn._path.c_str() + pos + 1;
                if (detail::equals_simple_insensitive(ext, GHC_PLATFORM_LITERAL("exe")) || detail::equals_simple_insensitive(ext, GHC_PLATFORM_LITERAL("cmd")) || detail::equals_simple_insensitive(ext, GHC_PLATFORM_LITERAL("bat")) ||
                    detail::equals_simple_insensitive(ext, GHC_PLATFORM_LITERAL("com"))) {
                    return true;
                }
            }
            return false;
        }
    }  // namespace detail
#endif


    //-----------------------------------------------------------------------------
    // [fs.path.query] query
    bool FilePath::empty() const noexcept {
        return _path.empty();
    }

    bool FilePath::has_root_name() const {
        return root_name_length() > 0;
    }

    bool FilePath::has_root_directory() const {
        auto rootLen = _prefixLength + root_name_length();
        return (_path.length() > rootLen && _path[rootLen] == preferred_separator);
    }

    bool FilePath::has_root_path() const {
        return has_root_name() || has_root_directory();
    }

    bool FilePath::has_relative_path() const {
        auto rootPathLen = _prefixLength + root_name_length() + (has_root_directory() ? 1 : 0);
        return rootPathLen < _path.length();
    }

    bool FilePath::has_parent_path() const {
        return !parent_path().empty();
    }

    bool FilePath::has_filename() const {
        return has_relative_path() && !filename().empty();
    }

    bool FilePath::has_stem() const {
        return !stem().empty();
    }

    bool FilePath::has_extension() const {
        return !extension().empty();
    }

    bool FilePath::is_absolute() const {
#ifdef OS_WIN
        return has_root_name() && has_root_directory();
#else
        return has_root_directory();
#endif
    }

    bool FilePath::is_relative() const {
        return !is_absolute();
    }

    //-----------------------------------------------------------------------------
    // [fs.path.gen] generation
    FilePath FilePath::lexically_normal() const {
        FilePath dest;
        bool lastDotDot = false;
        for (string_type s: *this) {
            if (s == ".") {
                dest /= "";
                continue;
            } else if (s == ".." && !dest.empty()) {
                auto root = root_path();
                if (dest == root) {
                    continue;
                } else if (*(--dest.end()) != "..") {
                    if (dest._path.back() == preferred_separator) {
                        dest._path.pop_back();
                    }
                    dest.remove_filename();
                    continue;
                }
            }
            if (!(s.empty() && lastDotDot)) {
                dest /= s;
            }
            lastDotDot = s == "..";
        }
        if (dest.empty()) {
            dest = ".";
        }
        return dest;
    }

    FilePath FilePath::lexically_relative(const FilePath &base) const {
        if (root_name() != base.root_name() || is_absolute() != base.is_absolute() ||
            (!has_root_directory() && base.has_root_directory())) {
            return FilePath();
        }
        const_iterator a = begin(), b = base.begin();
        while (a != end() && b != base.end() && *a == *b) {
            ++a;
            ++b;
        }
        if (a == end() && b == base.end()) {
            return FilePath(".");
        }
        int count = 0;
        for (const auto &element: input_iterator_range<const_iterator>(b, base.end())) {
            if (element != "." && element != "" && element != "..") {
                ++count;
            } else if (element == "..") {
                --count;
            }
        }
        if (count == 0 && (a == end() || a->empty())) {
            return FilePath(".");
        }
        if (count < 0) {
            return FilePath();
        }
        FilePath result;
        for (int i = 0; i < count; ++i) {
            result /= "..";
        }
        for (const auto &element: input_iterator_range<const_iterator>(a, end())) {
            result /= element;
        }
        return result;
    }

    FilePath FilePath::lexically_proximate(const FilePath &base) const {
        FilePath result = lexically_relative(base);
        return result.empty() ? *this : result;
    }

    //-----------------------------------------------------------------------------
    // [fs.path.itr] iterators
    FilePath::iterator::iterator() {}

    FilePath::iterator::iterator(const FilePath &p, const impl_string_type::const_iterator &pos)
            : _first(p._path.begin()), _last(p._path.end()),
              _prefix(_first + static_cast<string_type::difference_type>(p._prefixLength)),
              _root(p.has_root_directory() ? _first + static_cast<string_type::difference_type>(p._prefixLength +
                                                                                                p.root_name_length())
                                           : _last), _iter(pos) {
        if (pos != _last) {
            updateCurrent();
        }
    }


    FilePath::impl_string_type::const_iterator
    FilePath::iterator::increment(const FilePath::impl_string_type::const_iterator &pos) const {
        FilePath::impl_string_type::const_iterator i = pos;
        bool fromStart = i == _first || i == _prefix;
        if (i != _last) {
            if (fromStart && i == _first && _prefix > _first) {
                i = _prefix;
            } else if (*i++ == preferred_separator) {
                // we can only sit on a slash if it is a network name or a root
                if (i != _last && *i == preferred_separator) {
                    if (fromStart && !(i + 1 != _last && *(i + 1) == preferred_separator)) {
                        // leadind double slashes detected, treat this and the
                        // following until a slash as one unit
                        i = std::find(++i, _last, preferred_separator);
                    } else {
                        // skip redundant slashes
                        while (i != _last && *i == preferred_separator) {
                            ++i;
                        }
                    }
                }
            } else {
#ifdef OS_WIN
                if (fromStart && i != _last && *i == ':') {
                ++i;
            }
            else {
#else
                {
#endif
                    i = std::find(i, _last, preferred_separator);
                }
            }
        }
        return i;
    }

    FilePath::impl_string_type::const_iterator
    FilePath::iterator::decrement(const FilePath::impl_string_type::const_iterator &pos) const {
        FilePath::impl_string_type::const_iterator i = pos;
        if (i != _first) {
            --i;
            // if this is now the root slash or the trailing slash, we are done,
            // else check for network name
            if (i != _root && (pos != _last || *i != preferred_separator)) {
#ifdef OS_WIN
                static const impl_string_type seps = GHC_PLATFORM_LITERAL("\\:");
            i = std::find_first_of(std::reverse_iterator<FilePath::impl_string_type::const_iterator>(i), std::reverse_iterator<FilePath::impl_string_type::const_iterator>(_first), seps.begin(), seps.end()).base();
            if (i > _first && *i == ':') {
                i++;
            }
#else
                i = std::find(std::reverse_iterator<FilePath::impl_string_type::const_iterator>(i),
                              std::reverse_iterator<FilePath::impl_string_type::const_iterator>(_first),
                              preferred_separator).base();
#endif
                // Now we have to check if this is a network name
                if (i - _first == 2 && *_first == preferred_separator && *(_first + 1) == preferred_separator) {
                    i -= 2;
                }
            }
        }
        return i;
    }

    void FilePath::iterator::updateCurrent() {
        if ((_iter == _last) ||
            (_iter != _first && _iter != _last && (*_iter == preferred_separator && _iter != _root) &&
             (_iter + 1 == _last))) {
            _current.clear();
        } else {
            _current.assign(_iter, increment(_iter));
        }
    }

    FilePath::iterator &FilePath::iterator::operator++() {
        _iter = increment(_iter);
        while (_iter != _last &&                 // we didn't reach the end
               _iter != _root &&                 // this is not a root position
               *_iter == preferred_separator &&  // we are on a separator
               (_iter + 1) != _last              // the slash is not the last char
                ) {
            ++_iter;
        }
        updateCurrent();
        return *this;
    }

    FilePath::iterator FilePath::iterator::operator++(int) {
        FilePath::iterator i{*this};
        ++(*this);
        return i;
    }


    FilePath::iterator &FilePath::iterator::operator--() {
        _iter = decrement(_iter);
        updateCurrent();
        return *this;
    }

    FilePath::iterator FilePath::iterator::operator--(int) {
        auto i = *this;
        --(*this);
        return i;
    }

    bool FilePath::iterator::operator==(const FilePath::iterator &other) const {
        return _iter == other._iter;
    }

    bool FilePath::iterator::operator!=(const FilePath::iterator &other) const {
        return _iter != other._iter;
    }

    FilePath::iterator::reference FilePath::iterator::operator*() const {
        return _current;
    }

    FilePath::iterator::pointer FilePath::iterator::operator->() const {
        return &_current;
    }

    FilePath::iterator FilePath::begin() const {
        return iterator{*this, _path.begin()};
    }

    FilePath::iterator FilePath::end() const {
        return iterator{*this, _path.end()};
    }

    turbo::Result<FilePath> FilePath::from_string(string_view_type str) {
        TURBO_RETURN_NOT_OK(detail::validate_path(str));
        try {
            FilePath path(str);
            return path;
        } catch (const std::exception &e) {
            return turbo::invalid_argument_error(e.what());
        }
    }

    //-----------------------------------------------------------------------------
    // [fs.path.nonmember] path non-member functions
    void swap(FilePath &lhs, FilePath &rhs) noexcept {
        swap(lhs._path, rhs._path);
    }

    size_t hash_value(const FilePath &p) noexcept {
        return std::hash<std::string>()(p.generic_string());
    }


    bool operator==(const FilePath &lhs, const FilePath &rhs) noexcept {
        return lhs.compare(rhs) == 0;
    }

    bool operator!=(const FilePath &lhs, const FilePath &rhs) noexcept {
        return !(lhs == rhs);
    }

    bool operator<(const FilePath &lhs, const FilePath &rhs) noexcept {
        return lhs.compare(rhs) < 0;
    }

    bool operator<=(const FilePath &lhs, const FilePath &rhs) noexcept {
        return lhs.compare(rhs) <= 0;
    }

    bool operator>(const FilePath &lhs, const FilePath &rhs) noexcept {
        return lhs.compare(rhs) > 0;
    }

    bool operator>=(const FilePath &lhs, const FilePath &rhs) noexcept {
        return lhs.compare(rhs) >= 0;
    }

    FilePath operator/(const FilePath &lhs, const FilePath &rhs) {
        FilePath result(lhs);
        result /= rhs;
        return result;
    }

    //-----------------------------------------------------------------------------
    // [fs.class.filesystem_error] Class filesystem_error
    filesystem_error::filesystem_error(const std::string &what_arg, std::error_code ec)
            : std::system_error(ec, what_arg), _what_arg(what_arg), _ec(ec) {
    }

    filesystem_error::filesystem_error(const std::string &what_arg, const FilePath &p1, std::error_code ec)
            : std::system_error(ec, what_arg), _what_arg(what_arg), _ec(ec), _p1(p1) {
        if (!_p1.empty()) {
            _what_arg += ": '" + _p1.string() + "'";
        }
    }

    filesystem_error::filesystem_error(const std::string &what_arg, const FilePath &p1, const FilePath &p2,
                                       std::error_code ec)
            : std::system_error(ec, what_arg), _what_arg(what_arg), _ec(ec), _p1(p1), _p2(p2) {
        if (!_p1.empty()) {
            _what_arg += ": '" + _p1.string() + "'";
        }
        if (!_p2.empty()) {
            _what_arg += ", '" + _p2.string() + "'";
        }
    }

    const FilePath &filesystem_error::path1() const noexcept {
        return _p1;
    }

    const FilePath &filesystem_error::path2() const noexcept {
        return _p2;
    }

    const char *filesystem_error::what() const noexcept {
        return _what_arg.c_str();
    }
    //-----------------------------------------------------------------------------
    // [fs.op.funcs] filesystem operations
    namespace fs_detail {

        turbo::Status error_code_to_status(const std::string &what_arg, std::error_code ec) {
            static std::string nil_path = "nil";
            return make_status(ec.value(), what_arg);
        }


        turbo::Status error_code_to_status(const std::string &what_arg, const FilePath &p1, std::error_code ec) {
            static std::string nil_path = "nil";
            return make_status(ec.value(), what_arg, ": '", (p1.empty() ? nil_path : p1.string()), "'");
        }

        turbo::Status
        error_code_to_status(const std::string &what_arg, const FilePath &p1, const FilePath &p2, std::error_code ec) {
            static std::string nil_path = "nil";
            return make_status(ec.value(), what_arg, ": from'", (p1.empty() ? nil_path : p1.string()) + "' to '",
                               (p2.empty() ? nil_path : p2.string()), "'");
        }
    } // namespace fs_detail

    turbo::Result<FilePath> absolute(const FilePath &p) {
        std::error_code ec;
        FilePath result = absolute(p, ec);
        if (ec) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), p, ec);
        }
        return result;
    }

    FilePath absolute(const FilePath &p, std::error_code &ec) {
        ec.clear();
#ifdef OS_WIN
        if (p.empty()) {
        return absolute(current_path(ec), ec) / "";
    }
    ULONG size = ::GetFullPathNameW(GHC_NATIVEWP(p), 0, 0, 0);
    if (size) {
        std::vector<wchar_t> buf(size, 0);
        ULONG s2 = GetFullPathNameW(GHC_NATIVEWP(p), size, buf.data(), nullptr);
        if (s2 && s2 < size) {
            FilePath result = FilePath(std::wstring(buf.data(), s2));
            if (p.filename() == ".") {
                result /= ".";
            }
            return result;
        }
    }
    ec = detail::make_system_error();
    return FilePath();
#else
        FilePath base = current_path(ec);
        if (!ec) {
            if (p.empty()) {
                return base / p;
            }
            if (p.has_root_name()) {
                if (p.has_root_directory()) {
                    return p;
                } else {
                    return p.root_name() / base.root_directory() / base.relative_path() / p.relative_path();
                }
            } else {
                if (p.has_root_directory()) {
                    return base.root_name() / p;
                } else {
                    return base / p;
                }
            }
        }
        ec = detail::make_system_error();
        return FilePath{};
#endif
    }

    turbo::Result<FilePath> canonical(const FilePath &p) {
        std::error_code ec;
        auto result = canonical(p, ec);
        if (ec) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), p, ec);
        }
        return result;
    }


    FilePath canonical(const FilePath &p, std::error_code &ec) {
        if (p.empty()) {
            ec = detail::make_error_code(detail::portable_error::not_found);
            return FilePath{};
        }
        FilePath work = p.is_absolute() ? p : absolute(p, ec);
        FilePath result;

        auto fs = status(work, ec);
        if (ec) {
            return FilePath{};
        }
        if (fs.type() == FileType::not_found) {
            ec = detail::make_error_code(detail::portable_error::not_found);
            return FilePath{};
        }
        bool redo;
        do {
            auto rootPathLen = work._prefixLength + work.root_name_length() + (work.has_root_directory() ? 1 : 0);
            redo = false;
            result.clear();
            for (auto pe: work) {
                if (pe.empty() || pe == ".") {
                    continue;
                } else if (pe == "..") {
                    result = result.parent_path();
                    continue;
                } else if ((result / pe).string().length() <= rootPathLen) {
                    result /= pe;
                    continue;
                }
                auto sls = symlink_status(result / pe, ec);
                if (ec) {
                    return FilePath{};
                }
                if (is_symlink(sls)) {
                    redo = true;
                    auto target = read_symlink(result / pe, ec);
                    if (ec) {
                        return FilePath{};
                    }
                    if (target.is_absolute()) {
                        result = target;
                        continue;
                    } else {
                        result /= target;
                        continue;
                    }
                } else {
                    result /= pe;
                }
            }
            work = result;
        } while (redo);
        ec.clear();
        return result;
    }

    turbo::Status copy(const FilePath &from, const FilePath &to) {
        return copy(from, to, FileCopyOptions::none);
    }

    Status copy(const FilePath &from, const FilePath &to, FileCopyOptions options) {
        std::error_code ec;
        copy(from, to, options, ec);
        if (ec) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), from, to, ec);
        }
        return turbo::OkStatus();
    }


    void copy(const FilePath &from, const FilePath &to, std::error_code &ec) noexcept {
        copy(from, to, FileCopyOptions::none, ec);
    }

    void copy(const FilePath &from, const FilePath &to, FileCopyOptions options, std::error_code &ec) noexcept {
        std::error_code tec;
        FileStatus fs_from, fs_to;
        ec.clear();
        if ((options &
             (FileCopyOptions::skip_symlinks | FileCopyOptions::copy_symlinks | FileCopyOptions::create_symlinks)) !=
            FileCopyOptions::none) {
            fs_from = symlink_status(from, ec);
        } else {
            fs_from = status(from, ec);
        }
        if (!exists(fs_from)) {
            if (!ec) {
                ec = detail::make_error_code(detail::portable_error::not_found);
            }
            return;
        }
        if ((options & (FileCopyOptions::skip_symlinks | FileCopyOptions::create_symlinks)) != FileCopyOptions::none) {
            fs_to = symlink_status(to, tec);
        } else {
            fs_to = status(to, tec);
        }
        if (is_other(fs_from) || is_other(fs_to) || (is_directory(fs_from) && is_regular_file(fs_to)) ||
            (exists(fs_to) && equivalent(from, to, ec))) {
            ec = detail::make_error_code(detail::portable_error::invalid_argument);
        } else if (is_symlink(fs_from)) {
            if ((options & FileCopyOptions::skip_symlinks) == FileCopyOptions::none) {
                if (!exists(fs_to) && (options & FileCopyOptions::copy_symlinks) != FileCopyOptions::none) {
                    copy_symlink(from, to, ec);
                } else {
                    ec = detail::make_error_code(detail::portable_error::invalid_argument);
                }
            }
        } else if (is_regular_file(fs_from)) {
            if ((options & FileCopyOptions::directories_only) == FileCopyOptions::none) {
                if ((options & FileCopyOptions::create_symlinks) != FileCopyOptions::none) {
                    create_symlink(from.is_absolute() ? from : canonical(from, ec), to, ec);
                }
#ifndef OS_WEB
                else if ((options & FileCopyOptions::create_hard_links) != FileCopyOptions::none) {
                    create_hard_link(from, to, ec);
                }
#endif
                else if (is_directory(fs_to)) {
                    copy_file(from, to / from.filename(), options, ec);
                } else {
                    copy_file(from, to, options, ec);
                }
            }
        } else if (is_directory(fs_from) && (options & FileCopyOptions::create_symlinks) != FileCopyOptions::none) {
            ec = detail::make_error_code(detail::portable_error::is_a_directory);
        } else if (is_directory(fs_from) &&
                   (options == FileCopyOptions::none ||
                    (options & FileCopyOptions::recursive) != FileCopyOptions::none)) {
            if (!exists(fs_to)) {
                create_directory(to, from, ec);
                if (ec) {
                    return;
                }
            }
            for (auto iter = DirectoryIterator(from, ec); iter != DirectoryIterator(); iter.increment(ec)) {
                if (!ec) {
                    copy(iter->path(), to / iter->path().filename(), options | static_cast<FileCopyOptions>(0x8000),
                         ec);
                }
                if (ec) {
                    return;
                }
            }
        }
        return;
    }


    Result<bool> copy_file(const FilePath &from, const FilePath &to) {
        return copy_file(from, to, FileCopyOptions::none);
    }

    Result<bool> copy_file(const FilePath &from, const FilePath &to, FileCopyOptions option) {
        std::error_code ec;
        auto result = copy_file(from, to, option, ec);
        if (ec) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), from, to, ec);
        }
        return result;
    }


    bool copy_file(const FilePath &from, const FilePath &to, std::error_code &ec) noexcept {
        return copy_file(from, to, FileCopyOptions::none, ec);
    }

    bool copy_file(const FilePath &from, const FilePath &to, FileCopyOptions options, std::error_code &ec) noexcept {
        std::error_code tecf, tect;
        auto sf = status(from, tecf);
        auto st = status(to, tect);
        bool overwrite = false;
        ec.clear();
        if (!is_regular_file(sf)) {
            ec = tecf;
            return false;
        }
        if (exists(st)) {
            if ((options & FileCopyOptions::skip_existing) == FileCopyOptions::skip_existing) {
                return false;
            }
            if (!is_regular_file(st) || equivalent(from, to, ec) ||
                (options & (FileCopyOptions::overwrite_existing | FileCopyOptions::update_existing)) ==
                FileCopyOptions::none) {
                ec = tect ? tect : detail::make_error_code(detail::portable_error::exists);
                return false;
            }
            if ((options & FileCopyOptions::update_existing) == FileCopyOptions::update_existing) {
                auto from_time = last_write_time(from, ec);
                if (ec) {
                    ec = detail::make_system_error();
                    return false;
                }
                auto to_time = last_write_time(to, ec);
                if (ec) {
                    ec = detail::make_system_error();
                    return false;
                }
                if (from_time <= to_time) {
                    return false;
                }
            }
            overwrite = true;
        }
#ifdef OS_WIN
        if (!::CopyFileW(GHC_NATIVEWP(from), GHC_NATIVEWP(to), !overwrite)) {
        ec = detail::make_system_error();
        return false;
    }
    return true;
#else
        std::vector<char> buffer(16384, '\0');
        int in = -1, out = -1;
        if ((in = ::open(from.c_str(), O_RDONLY)) < 0) {
            ec = detail::make_system_error();
            return false;
        }
        int mode = O_CREAT | O_WRONLY | O_TRUNC;
        if (!overwrite) {
            mode |= O_EXCL;
        }
        if ((out = ::open(to.c_str(), mode, static_cast<int>(sf.permissions() & FilePerms::all))) < 0) {
            ec = detail::make_system_error();
            ::close(in);
            return false;
        }
        if (st.permissions() != sf.permissions()) {
            if (::fchmod(out, static_cast<mode_t>(sf.permissions() & FilePerms::all)) != 0) {
                ec = detail::make_system_error();
                ::close(in);
                ::close(out);
                return false;
            }
        }
        ssize_t br, bw;
        while (true) {
            do { br = ::read(in, buffer.data(), buffer.size()); } while (errno == EINTR && !br);
            if (!br) {
                break;
            }
            if (br < 0) {
                ec = detail::make_system_error();
                ::close(in);
                ::close(out);
                return false;
            }
            ssize_t offset = 0;
            do {
                if ((bw = ::write(out, buffer.data() + offset, static_cast<size_t>(br))) > 0) {
                    br -= bw;
                    offset += bw;
                } else if (bw < 0 && errno != EINTR) {
                    ec = detail::make_system_error();
                    ::close(in);
                    ::close(out);
                    return false;
                }
            } while (br);
        }
        ::close(in);
        ::close(out);
        return true;
#endif
    }


    Status copy_symlink(const FilePath &existing_symlink, const FilePath &new_symlink) {
        std::error_code ec;
        copy_symlink(existing_symlink, new_symlink, ec);
        if (ec) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), existing_symlink, new_symlink,
                                                   ec);
        }
        return turbo::OkStatus();
    }

    void copy_symlink(const FilePath &existing_symlink, const FilePath &new_symlink, std::error_code &ec) noexcept {
        ec.clear();
        auto to = read_symlink(existing_symlink, ec);
        if (!ec) {
            if (exists(to, ec) && is_directory(to, ec)) {
                create_directory_symlink(to, new_symlink, ec);
            } else {
                create_symlink(to, new_symlink, ec);
            }
        }
    }


    Result<bool> create_directories(const FilePath &p) {
        std::error_code ec;
        auto result = create_directories(p, ec);
        if (ec) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), p, ec);
        }
        return result;
    }

    bool create_directories(const FilePath &p, std::error_code &ec) noexcept {
        FilePath current;
        ec.clear();
        bool didCreate = false;
        auto rootPathLen = p._prefixLength + p.root_name_length() + (p.has_root_directory() ? 1 : 0);
        current = p.native().substr(0, rootPathLen);
        FilePath folders(p._path.substr(rootPathLen));
        for (FilePath::string_type part: folders) {
            current /= part;
            std::error_code tec;
            auto fs = status(current, tec);
            if (tec && fs.type() != FileType::not_found) {
                ec = tec;
                return false;
            }
            if (!exists(fs)) {
                create_directory(current, ec);
                if (ec) {
                    std::error_code tmp_ec;
                    if (is_directory(current, tmp_ec)) {
                        ec.clear();
                    } else {
                        return false;
                    }
                }
                didCreate = true;
            } else if (!is_directory(fs)) {
                ec = detail::make_error_code(detail::portable_error::exists);
                return false;
            }
        }
        return didCreate;
    }

    Result<bool> create_directory(const FilePath &p) {
        std::error_code ec;
        auto result = create_directory(p, FilePath{}, ec);
        if (ec) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), p, ec);
        }
        return result;
    }

    bool create_directory(const FilePath &p, std::error_code &ec) noexcept {
        return create_directory(p, FilePath{}, ec);
    }

    Result<bool> create_directory(const FilePath &p, const FilePath &attributes) {
        std::error_code ec;
        auto result = create_directory(p, attributes, ec);
        if (ec) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), p, ec);
        }
        return result;
    }


    bool create_directory(const FilePath &p, const FilePath &attributes, std::error_code &ec) noexcept {
        std::error_code tec;
        ec.clear();
        auto fs = status(p, tec);
        if (status_known(fs) && exists(fs) && is_directory(fs)) {
            return false;
        }
#ifdef OS_WIN
        if (!attributes.empty()) {
        if (!::CreateDirectoryExW(GHC_NATIVEWP(attributes), GHC_NATIVEWP(p), nullptr)) {
            ec = detail::make_system_error();
            return false;
        }
    }
    else if (!::CreateDirectoryW(GHC_NATIVEWP(p), nullptr)) {
        ec = detail::make_system_error();
        return false;
    }
#else
        ::mode_t attribs = static_cast<mode_t>(FilePerms::all);
        if (!attributes.empty()) {
            struct ::stat fileStat;
            if (::stat(attributes.c_str(), &fileStat) != 0) {
                ec = detail::make_system_error();
                return false;
            }
            attribs = fileStat.st_mode;
        }
        if (::mkdir(p.c_str(), attribs) != 0) {
            ec = detail::make_system_error();
            return false;
        }
#endif
        return true;
    }

    Status create_directory_symlink(const FilePath &to, const FilePath &new_symlink) {
        std::error_code ec;
        create_directory_symlink(to, new_symlink, ec);
        if (ec) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), to, new_symlink, ec);
        }
        return turbo::OkStatus();
    }

    void create_directory_symlink(const FilePath &to, const FilePath &new_symlink, std::error_code &ec) noexcept {
        detail::create_symlink(to, new_symlink, true, ec);
    }

#ifndef OS_WEB

    Status create_hard_link(const FilePath &to, const FilePath &new_hard_link) {
        std::error_code ec;
        create_hard_link(to, new_hard_link, ec);
        if (ec) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), to, new_hard_link, ec);
        }
        return turbo::OkStatus();
    }

    void create_hard_link(const FilePath &to, const FilePath &new_hard_link, std::error_code &ec) noexcept {
        detail::create_hardlink(to, new_hard_link, ec);
    }

#endif

    Status create_symlink(const FilePath &to, const FilePath &new_symlink) {
        std::error_code ec;
        create_symlink(to, new_symlink, ec);
        if (ec) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), to, new_symlink, ec);
        }
        return turbo::OkStatus();
    }

    void create_symlink(const FilePath &to, const FilePath &new_symlink, std::error_code &ec) noexcept {
        detail::create_symlink(to, new_symlink, false, ec);
    }


    Result<FilePath> current_path() {
        std::error_code ec;
        auto result = current_path(ec);
        if (ec) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), ec);
        }
        return result;
    }


    FilePath current_path(std::error_code &ec) {
        ec.clear();
#ifdef OS_WIN
        DWORD pathlen = ::GetCurrentDirectoryW(0, 0);
        std::unique_ptr<wchar_t[]> buffer(new wchar_t[size_t(pathlen) + 1]);
        if (::GetCurrentDirectoryW(pathlen, buffer.get()) == 0) {
            ec = detail::make_system_error();
            return FilePath{};
        }
        return FilePath(std::wstring(buffer.get()), FilePath::native_format);
#elif defined(__GLIBC__)
        std::unique_ptr<char, decltype(&std::free)> buffer{::getcwd(nullptr, 0), std::free};
        if (buffer == nullptr) {
            ec = detail::make_system_error();
            return FilePath{};
        }
        return FilePath(buffer.get());
#else
        size_t pathlen = static_cast<size_t>(std::max(int(::pathconf(".", _PC_PATH_MAX)), int(PATH_MAX)));
            std::unique_ptr<char[]> buffer(new char[pathlen + 1]);
            if (::getcwd(buffer.get(), pathlen) == nullptr) {
                ec = detail::make_system_error();
                return FilePath{};
            }
            return FilePath(buffer.get());
#endif
    }


    Status current_path(const FilePath &p) {
        std::error_code ec;
        current_path(p, ec);
        if (ec) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), p, ec);
        }
        return turbo::OkStatus();
    }


    void current_path(const FilePath &p, std::error_code &ec) noexcept {
        ec.clear();
#ifdef OS_WIN
        if (!::SetCurrentDirectoryW(GHC_NATIVEWP(p))) {
            ec = detail::make_system_error();
        }
#else
        if (::chdir(p.string().c_str()) == -1) {
            ec = detail::make_system_error();
        }
#endif
    }

    bool exists(FileStatus s) noexcept {
        return status_known(s) && s.type() != FileType::not_found;
    }

    Result<bool> exists(const FilePath &p) {
        TURBO_MOVE_OR_RAISE(auto ps, status(p));
        return exists(ps);
    }

    bool exists(const FilePath &p, std::error_code &ec) noexcept {
        FileStatus s = status(p, ec);
        if (status_known(s)) {
            ec.clear();
        }
        return exists(s);
    }

    Result<bool> equivalent(const FilePath &p1, const FilePath &p2) {
        std::error_code ec;
        bool result = equivalent(p1, p2, ec);
        if (ec) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), p1, p2, ec);
        }
        return result;
    }

    bool equivalent(const FilePath &p1, const FilePath &p2, std::error_code &ec) noexcept {
        ec.clear();
#ifdef OS_WIN
        detail::unique_handle file1(::CreateFileW(GHC_NATIVEWP(p1), 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0));
        auto e1 = ::GetLastError();
        detail::unique_handle file2(::CreateFileW(GHC_NATIVEWP(p2), 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0));
        if (!file1 || !file2) {
            ec = detail::make_system_error(e1 ? e1 : ::GetLastError());
            return false;
        }
        BY_HANDLE_FILE_INFORMATION inf1, inf2;
        if (!::GetFileInformationByHandle(file1.get(), &inf1)) {
            ec = detail::make_system_error();
            return false;
        }
        if (!::GetFileInformationByHandle(file2.get(), &inf2)) {
            ec = detail::make_system_error();
            return false;
        }
        return inf1.ftLastWriteTime.dwLowDateTime == inf2.ftLastWriteTime.dwLowDateTime && inf1.ftLastWriteTime.dwHighDateTime == inf2.ftLastWriteTime.dwHighDateTime && inf1.nFileIndexHigh == inf2.nFileIndexHigh && inf1.nFileIndexLow == inf2.nFileIndexLow &&
               inf1.nFileSizeHigh == inf2.nFileSizeHigh && inf1.nFileSizeLow == inf2.nFileSizeLow && inf1.dwVolumeSerialNumber == inf2.dwVolumeSerialNumber;
#else
        struct ::stat s1, s2;
        auto rc1 = ::stat(p1.c_str(), &s1);
        auto e1 = errno;
        auto rc2 = ::stat(p2.c_str(), &s2);
        if (rc1 || rc2) {
            ec = detail::make_system_error(e1 ? e1 : errno);
            return false;
        }
        return s1.st_dev == s2.st_dev && s1.st_ino == s2.st_ino && s1.st_size == s2.st_size &&
               s1.st_mtime == s2.st_mtime;
#endif
    }

#ifdef TURBO_FS_WITH_EXCEPTIONS

    Result<uintmax_t> file_size(const FilePath &p) {
        std::error_code ec;
        auto result = file_size(p, ec);
        if (ec) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), p, ec);
        }
        return result;
    }

#endif

    uintmax_t file_size(const FilePath &p, std::error_code &ec) noexcept {
        ec.clear();
#ifdef OS_WIN
        WIN32_FILE_ATTRIBUTE_DATA attr;
    if (!GetFileAttributesExW(GHC_NATIVEWP(p), GetFileExInfoStandard, &attr)) {
        ec = detail::make_system_error();
        return static_cast<uintmax_t>(-1);
    }
    return static_cast<uintmax_t>(attr.nFileSizeHigh) << (sizeof(attr.nFileSizeHigh) * 8) | attr.nFileSizeLow;
#else
        struct ::stat fileStat;
        if (::stat(p.c_str(), &fileStat) == -1) {
            ec = detail::make_system_error();
            return static_cast<uintmax_t>(-1);
        }
        return static_cast<uintmax_t>(fileStat.st_size);
#endif
    }


#ifndef OS_WEB

    Result<uintmax_t> hard_link_count(const FilePath &p) {
        std::error_code ec;
        auto result = hard_link_count(p, ec);
        if (ec) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), p, ec);
        }
        return result;
    }


    uintmax_t hard_link_count(const FilePath &p, std::error_code &ec) noexcept {
        ec.clear();
#ifdef OS_WIN
        uintmax_t result = static_cast<uintmax_t>(-1);
        detail::unique_handle file(::CreateFileW(GHC_NATIVEWP(p), 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0));
        BY_HANDLE_FILE_INFORMATION inf;
        if (!file) {
            ec = detail::make_system_error();
        }
        else {
            if (!::GetFileInformationByHandle(file.get(), &inf)) {
                ec = detail::make_system_error();
            }
            else {
                result = inf.nNumberOfLinks;
            }
        }
        return result;
#else
        uintmax_t result = 0;
        FileStatus fs = detail::status_ex(p, ec, nullptr, nullptr, &result, nullptr);
        if (fs.type() == FileType::not_found) {
            ec = detail::make_error_code(detail::portable_error::not_found);
        }
        return ec ? static_cast<uintmax_t>(-1) : result;
#endif
    }

#endif

    bool is_block_file(FileStatus s) noexcept {
        return s.type() == FileType::block;
    }

    Result<bool> is_block_file(const FilePath &p) {
        TURBO_MOVE_OR_RAISE(auto ps, status(p));
        return is_block_file(ps);
    }


    bool is_block_file(const FilePath &p, std::error_code &ec) noexcept {
        return is_block_file(status(p, ec));
    }

    bool is_character_file(FileStatus s) noexcept {
        return s.type() == FileType::character;
    }

    Result<bool> is_character_file(const FilePath &p) {
        TURBO_MOVE_OR_RAISE(auto ps, status(p));
        return is_character_file(ps);
    }


    bool is_character_file(const FilePath &p, std::error_code &ec) noexcept {
        return is_character_file(status(p, ec));
    }

    bool is_directory(FileStatus s) noexcept {
        return s.type() == FileType::directory;
    }

    Result<bool> is_directory(const FilePath &p) {
        TURBO_MOVE_OR_RAISE(auto ps, status(p));
        return is_directory(ps);
    }


    bool is_directory(const FilePath &p, std::error_code &ec) noexcept {
        return is_directory(status(p, ec));
    }

    Result<bool> is_empty(const FilePath &p) {
        TURBO_MOVE_OR_RAISE(auto isd, is_directory(p));
        if (isd) {
            std::error_code ec;
            auto it = DirectoryIterator(p, ec);
            if (ec) {
                return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), p, ec);
            }
            return it == DirectoryIterator();
        } else {
            TURBO_MOVE_OR_RAISE(auto fsize, file_size(p));
            return fsize == 0;
        }
    }

    bool is_empty(const FilePath &p, std::error_code &ec) noexcept {
        auto fs = status(p, ec);
        if (ec) {
            return false;
        }
        if (is_directory(fs)) {
            DirectoryIterator iter(p, ec);
            if (ec) {
                return false;
            }
            return iter == DirectoryIterator();
        } else {
            auto sz = file_size(p, ec);
            if (ec) {
                return false;
            }
            return sz == 0;
        }
    }

    bool is_fifo(FileStatus s) noexcept {
        return s.type() == FileType::fifo;
    }

    Result<bool> is_fifo(const FilePath &p) {
        TURBO_MOVE_OR_RAISE(auto ps, status(p));
        return is_fifo(ps);
    }

    bool is_fifo(const FilePath &p, std::error_code &ec) noexcept {
        return is_fifo(status(p, ec));
    }

    bool is_other(FileStatus s) noexcept {
        return exists(s) && !is_regular_file(s) && !is_directory(s) && !is_symlink(s);
    }

    Result<bool> is_other(const FilePath &p) {
        TURBO_MOVE_OR_RAISE(auto ps, status(p));
        return is_other(ps);
    }


    bool is_other(const FilePath &p, std::error_code &ec) noexcept {
        return is_other(status(p, ec));
    }

    bool is_regular_file(FileStatus s) noexcept {
        return s.type() == FileType::regular;
    }

    Result<bool> is_regular_file(const FilePath &p) {
        TURBO_MOVE_OR_RAISE(auto ps, status(p));
        return is_regular_file(ps);
    }


    bool is_regular_file(const FilePath &p, std::error_code &ec) noexcept {
        return is_regular_file(status(p, ec));
    }

    bool is_socket(FileStatus s) noexcept {
        return s.type() == FileType::socket;
    }

    Result<bool> is_socket(const FilePath &p) {
        TURBO_MOVE_OR_RAISE(auto ps, status(p));
        return is_socket(ps);
    }

    bool is_socket(const FilePath &p, std::error_code &ec) noexcept {
        return is_socket(status(p, ec));
    }

    bool is_symlink(FileStatus s) noexcept {
        return s.type() == FileType::symlink;
    }

    Result<bool> is_symlink(const FilePath &p) {
        TURBO_MOVE_OR_RAISE(auto ps, symlink_status(p));
        return is_symlink(ps);
    }

    bool is_symlink(const FilePath &p, std::error_code &ec) noexcept {
        return is_symlink(symlink_status(p, ec));
    }

    Result<Time> last_write_time(const FilePath &p) {
        std::error_code ec;
        auto result = last_write_time(p, ec);
        if (ec) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), p, ec);
        }
        return result;
    }


    Time last_write_time(const FilePath &p, std::error_code &ec) noexcept {
        time_t result = 0;
        ec.clear();
        FileStatus fs = detail::status_ex(p, ec, nullptr, nullptr, nullptr, &result);
        return ec ? Time() : Time::from_time_t(result);
    }

    Status last_write_time(const FilePath &p, Time new_time) {
        std::error_code ec;
        last_write_time(p, new_time, ec);
        if (ec) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), p, ec);
        }
        return turbo::OkStatus();
    }


    void last_write_time(const FilePath &p, Time new_time, std::error_code &ec) noexcept {
        ec.clear();
#ifdef OS_WIN
        detail::unique_handle file(::CreateFileW(GHC_NATIVEWP(p), FILE_WRITE_ATTRIBUTES, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr));
    FILETIME ft;
    auto tt = std::chrono::duration_cast<std::chrono::microseconds>(d).count() * 10 + 116444736000000000;
    ft.dwLowDateTime = static_cast<DWORD>(tt);
    ft.dwHighDateTime = static_cast<DWORD>(tt >> 32);
    if (!::SetFileTime(file.get(), 0, 0, &ft)) {
        ec = detail::make_system_error();
    }
#elif defined(OS_MACOSX) && \
    (__MAC_OS_X_VERSION_MIN_REQUIRED && __MAC_OS_X_VERSION_MIN_REQUIRED < 101300 \
 || __IPHONE_OS_VERSION_MIN_REQUIRED && __IPHONE_OS_VERSION_MIN_REQUIRED < 110000 \
 || __TV_OS_VERSION_MIN_REQUIRED && __TVOS_VERSION_MIN_REQUIRED < 110000 \
 || __WATCH_OS_VERSION_MIN_REQUIRED && __WATCHOS_VERSION_MIN_REQUIRED < 40000)
        struct ::stat fs;
    if (::stat(p.c_str(), &fs) == 0) {
        struct ::timeval tv[2];
        tv[0].tv_sec = fs.st_atimespec.tv_sec;
        tv[0].tv_usec = static_cast<int>(fs.st_atimespec.tv_nsec / 1000);
        times[1] = turbo::Time::to_timeval(new_time);
        if (::utimes(p.c_str(), tv) == 0) {
            return;
        }
    }
    ec = detail::make_system_error();
    return;
#else
#ifndef UTIME_OMIT
#define UTIME_OMIT ((1l << 30) - 2l)
#endif
        struct ::timespec times[2];
        times[0].tv_sec = 0;
        times[0].tv_nsec = UTIME_OMIT;
        times[1] = turbo::Time::to_timespec(new_time);
#if defined(__ANDROID_API__) && __ANDROID_API__ < 12
        if (syscall(__NR_utimensat, AT_FDCWD, p.c_str(), times, AT_SYMLINK_NOFOLLOW) != 0) {
#else
        if (::utimensat((int) AT_FDCWD, p.c_str(), times, AT_SYMLINK_NOFOLLOW) != 0) {
#endif
            ec = detail::make_system_error();
        }
        return;
#endif
    }

    Status permissions(const FilePath &p, FilePerms prms, FilePermOptions opts) {
        std::error_code ec;
        permissions(p, prms, opts, ec);
        if (ec) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), p, ec);
        }
        return turbo::OkStatus();
    }

    std::function<bool(const DirectoryEntry &)> all_allow = [](const DirectoryEntry &) {
        return true;
    };


    template<typename T>
    TURBO_EXPORT Result<std::vector<FilePath>> list_files_template(const DirSelector &p) {
        std::error_code ec;
        std::vector<FilePath> result;
        std::function<bool(const DirectoryEntry &)> &filter = all_allow;
        if(p.filter) {
            filter = p.filter;
        }
        TURBO_MOVE_OR_RAISE(auto itr, T::create(p.base_dir, p.dir_options));
        size_t cnt = 0;
        while (!ec && itr != T() && cnt < p.max_recursion) {
            TURBO_MOVE_OR_RAISE(auto fst, itr->status());
            if(TURBO_LIKELY(filter(*itr))) {
                if (is_directory(fst) && p.include_dir) {
                    result.emplace_back(itr->path());
                    ++cnt;
                }
                if (is_regular_file(fst) && p.include_file) {
                    result.emplace_back(itr->path());
                    ++cnt;
                }
            }
            itr.increment(ec);
        }
        if (ec) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), p.base_dir, ec);
        }
        if(p.sort) {
            std::sort(result.begin(), result.end());
        }
        return result;
    }


    TURBO_EXPORT Result<std::vector<FilePath>> list_files(const DirSelector &p) {
        TURBO_MOVE_OR_RAISE(auto ext, turbo::exists(p.base_dir));
        if (!ext) {
            if (p.allow_not_found) {
                return std::vector<FilePath>{};
            } else {
                return io_error("path not exists: ", p.base_dir.c_str());
            }
        }
        if (p.recursive) {
            return list_files_template<RecursiveDirectoryIterator>(p);
        } else {
            return list_files_template<DirectoryIterator>(p);
        }
    }

    TURBO_EXPORT Result<std::vector<FilePath>> list_files(const FilePath &p, bool recursive) {
        DirSelector dp;
        dp.base_dir = p;
        dp.recursive = recursive;
        return list_files(dp);
    }

    template<typename T>
    TURBO_EXPORT Result<std::vector<DirectoryEntry>> list_entry_template(const DirSelector &p) {
        std::error_code ec;
        std::vector<DirectoryEntry> result;
        std::function<bool(const DirectoryEntry &)> &filter = all_allow;
        if(p.filter) {
            filter = p.filter;
        }
        TURBO_MOVE_OR_RAISE(auto itr, T::create(p.base_dir, p.dir_options));
        size_t cnt = 0;
        while (!ec && itr != T() && cnt < p.max_recursion) {
            TURBO_MOVE_OR_RAISE(auto fst, itr->status());
            if(TURBO_LIKELY(filter(*itr))) {
                if (is_directory(fst) && p.include_dir) {
                    result.emplace_back(*itr);
                    ++cnt;
                }
                if (is_regular_file(fst) && p.include_file) {
                    result.emplace_back(*itr);
                    ++cnt;
                }
            }
            itr.increment(ec);
        }
        if (ec) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), p.base_dir, ec);
        }
        if(p.sort) {
            std::sort(result.begin(), result.end());
        }
        return result;
    }

    TURBO_EXPORT Result<std::vector<DirectoryEntry>> list_entry(const DirSelector &p) {
        TURBO_MOVE_OR_RAISE(auto ext, turbo::exists(p.base_dir));
        if (!ext) {
            if (p.allow_not_found) {
                return std::vector<DirectoryEntry>{};
            } else {
                return io_error("path not exists: ", p.base_dir.c_str());
            }
        }
        if (p.recursive) {
            return list_entry_template<RecursiveDirectoryIterator>(p);
        } else {
            return list_entry_template<DirectoryIterator>(p);
        }
    }

    TURBO_EXPORT Result<std::vector<DirectoryEntry>> list_entry(const FilePath &p, bool recursive) {
        DirSelector dp;
        dp.base_dir = p;
        dp.recursive = recursive;
        return list_entry(dp);
    }

    void permissions(const FilePath &p, FilePerms prms, std::error_code &ec) noexcept {
        permissions(p, prms, FilePermOptions::replace, ec);
    }


    void permissions(const FilePath &p, FilePerms prms, FilePermOptions opts, std::error_code &ec) noexcept {
        if (static_cast<int>(opts & (FilePermOptions::replace | FilePermOptions::add | FilePermOptions::remove)) == 0) {
            ec = detail::make_error_code(detail::portable_error::invalid_argument);
            return;
        }
        auto fs = symlink_status(p, ec);
        if ((opts & FilePermOptions::replace) != FilePermOptions::replace) {
            if ((opts & FilePermOptions::add) == FilePermOptions::add) {
                prms = fs.permissions() | prms;
            } else {
                prms = fs.permissions() & ~prms;
            }
        }
#ifdef OS_WIN
#ifdef __GNUC__
        auto oldAttr = GetFileAttributesW(GHC_NATIVEWP(p));
        if (oldAttr != INVALID_FILE_ATTRIBUTES) {
            DWORD newAttr = ((prms & FilePerms::owner_write) == FilePerms::owner_write) ? oldAttr & ~(static_cast<DWORD>(FILE_ATTRIBUTE_READONLY)) : oldAttr | FILE_ATTRIBUTE_READONLY;
            if (oldAttr == newAttr || SetFileAttributesW(GHC_NATIVEWP(p), newAttr)) {
                return;
            }
        }
        ec = detail::make_system_error();
#else
        int mode = 0;
        if ((prms & FilePerms::owner_read) == FilePerms::owner_read) {
            mode |= _S_IREAD;
        }
        if ((prms & FilePerms::owner_write) == FilePerms::owner_write) {
            mode |= _S_IWRITE;
        }
        if (::_wchmod(p.wstring().c_str(), mode) != 0) {
            ec = detail::make_system_error();
        }
#endif
#else
        if ((opts & FilePermOptions::nofollow) != FilePermOptions::nofollow) {
            if (::chmod(p.c_str(), static_cast<mode_t>(prms)) != 0) {
                ec = detail::make_system_error();
            }
        }
#endif
    }

#ifdef TURBO_FS_WITH_EXCEPTIONS

    FilePath proximate(const FilePath &p, std::error_code &ec) {
        auto cp = current_path(ec);
        if (!ec) {
            return proximate(p, cp, ec);
        }
        return FilePath{};
    }

#endif


    Result<FilePath> proximate(const FilePath &p, const FilePath &base) {
        TURBO_MOVE_OR_RAISE(auto fp, weakly_canonical(p));
        TURBO_MOVE_OR_RAISE(auto fb, weakly_canonical(base));
        return fp.lexically_proximate(fb);
    }

    FilePath proximate(const FilePath &p, const FilePath &base, std::error_code &ec) {
        return weakly_canonical(p, ec).lexically_proximate(weakly_canonical(base, ec));
    }

    Result<FilePath> read_symlink(const FilePath &p) {
        std::error_code ec;
        auto result = read_symlink(p, ec);
        if (ec) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), p, ec);
        }
        return result;
    }


    FilePath read_symlink(const FilePath &p, std::error_code &ec) {
        FileStatus fs = symlink_status(p, ec);
        if (fs.type() != FileType::symlink) {
            ec = detail::make_error_code(detail::portable_error::invalid_argument);
            return FilePath();
        }
        auto result = detail::resolveSymlink(p, ec);
        return ec ? FilePath() : result;
    }

    FilePath relative(const FilePath &p, std::error_code &ec) {
        return relative(p, current_path(ec), ec);
    }

    Result<FilePath> relative(const FilePath &p, const FilePath &base) {
        TURBO_MOVE_OR_RAISE(auto fp, weakly_canonical(p));
        TURBO_MOVE_OR_RAISE(auto fb, weakly_canonical(base));
        return fp.lexically_relative(fb);
    }


    FilePath relative(const FilePath &p, const FilePath &base, std::error_code &ec) {
        return weakly_canonical(p, ec).lexically_relative(weakly_canonical(base, ec));
    }

    Result<bool> remove(const FilePath &p) {
        std::error_code ec;
        auto result = remove(p, ec);
        if (ec) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), p, ec);
        }
        return result;
    }

    bool remove(const FilePath &p, std::error_code &ec) noexcept {
        ec.clear();
#ifdef OS_WIN
#ifdef GHC_USE_WCHAR_T
        auto cstr = p.c_str();
#else
        std::wstring np = detail::fromUtf8<std::wstring>(p.u8string());
        auto cstr = np.c_str();
#endif
        DWORD attr = GetFileAttributesW(cstr);
        if (attr == INVALID_FILE_ATTRIBUTES) {
            auto error = ::GetLastError();
            if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND) {
                return false;
            }
            ec = detail::make_system_error(error);
        }
        else if (attr & FILE_ATTRIBUTE_READONLY) {
            auto new_attr = attr & ~static_cast<DWORD>(FILE_ATTRIBUTE_READONLY);
            if (!SetFileAttributesW(cstr, new_attr)) {
                auto error = ::GetLastError();
                ec = detail::make_system_error(error);
            }
        }
        if (!ec) {
            if (attr & FILE_ATTRIBUTE_DIRECTORY) {
                if (!RemoveDirectoryW(cstr)) {
                    ec = detail::make_system_error();
                }
            }
            else {
                if (!DeleteFileW(cstr)) {
                    ec = detail::make_system_error();
                }
            }
        }
#else
        if (::remove(p.c_str()) == -1) {
            auto error = errno;
            if (error == ENOENT) {
                return false;
            }
            ec = detail::make_system_error();
        }
#endif
        return ec ? false : true;
    }

#ifdef TURBO_FS_WITH_EXCEPTIONS

    Result<uintmax_t> remove_all(const FilePath &p) {
        std::error_code ec;
        auto result = remove_all(p, ec);
        if (ec) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), p, ec);
        }
        return result;
    }

#endif

    uintmax_t remove_all(const FilePath &p, std::error_code &ec) noexcept {
        ec.clear();
        uintmax_t count = 0;
        if (p == "/") {
            ec = detail::make_error_code(detail::portable_error::not_supported);
            return static_cast<uintmax_t>(-1);
        }
        std::error_code tec;
        auto fs = symlink_status(p, tec);
        if (exists(fs) && is_directory(fs)) {
            for (auto iter = DirectoryIterator(p, ec); iter != DirectoryIterator(); iter.increment(ec)) {
                if (ec && !detail::is_not_found_error(ec)) {
                    break;
                }
                bool is_symlink_result = iter->is_symlink(ec);
                if (ec)
                    return static_cast<uintmax_t>(-1);
                if (!is_symlink_result && iter->is_directory(ec)) {
                    count += remove_all(iter->path(), ec);
                    if (ec) {
                        return static_cast<uintmax_t>(-1);
                    }
                } else {
                    if (!ec) {
                        remove(iter->path(), ec);
                    }
                    if (ec) {
                        return static_cast<uintmax_t>(-1);
                    }
                    ++count;
                }
            }
        }
        if (!ec) {
            if (remove(p, ec)) {
                ++count;
            }
        }
        if (ec) {
            return static_cast<uintmax_t>(-1);
        }
        return count;
    }

    Status rename(const FilePath &from, const FilePath &to) {
        std::error_code ec;
        rename(from, to, ec);
        if (ec) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), from, to, ec);
        }
        return turbo::OkStatus();
    }

    void rename(const FilePath &from, const FilePath &to, std::error_code &ec) noexcept {
        ec.clear();
#ifdef OS_WIN
        if (from != to) {
        if (!MoveFileExW(GHC_NATIVEWP(from), GHC_NATIVEWP(to), (DWORD)MOVEFILE_REPLACE_EXISTING)) {
            ec = detail::make_system_error();
        }
    }
#else
        if (from != to) {
            if (::rename(from.c_str(), to.c_str()) != 0) {
                ec = detail::make_system_error();
            }
        }
#endif
    }

    Status resize_file(const FilePath &p, uintmax_t size) {
        std::error_code ec;
        resize_file(p, size, ec);
        if (ec) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), p, ec);
        }
        return turbo::OkStatus();
    }

    void resize_file(const FilePath &p, uintmax_t size, std::error_code &ec) noexcept {
        ec.clear();
#ifdef OS_WIN
        LARGE_INTEGER lisize;
    lisize.QuadPart = static_cast<LONGLONG>(size);
    if (lisize.QuadPart < 0) {
#ifdef ERROR_FILE_TOO_LARGE
        ec = detail::make_system_error(ERROR_FILE_TOO_LARGE);
#else
        ec = detail::make_system_error(223);
#endif
        return;
    }
    detail::unique_handle file(CreateFileW(GHC_NATIVEWP(p), GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr));
    if (!file) {
        ec = detail::make_system_error();
    }
    else if (SetFilePointerEx(file.get(), lisize, nullptr, FILE_BEGIN) == 0 || SetEndOfFile(file.get()) == 0) {
        ec = detail::make_system_error();
    }
#else
        if (::truncate(p.c_str(), static_cast<off_t>(size)) != 0) {
            ec = detail::make_system_error();
        }
#endif
    }

    Result<FileSpaceInfo> space(const FilePath &p) {
        std::error_code ec;
        auto result = space(p, ec);
        if (ec) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), p, ec);
        }
        return result;
    }

    FileSpaceInfo space(const FilePath &p, std::error_code &ec) noexcept {
        ec.clear();
#ifdef OS_WIN
        ULARGE_INTEGER freeBytesAvailableToCaller = {{ 0, 0 }};
        ULARGE_INTEGER totalNumberOfBytes = {{ 0, 0 }};
        ULARGE_INTEGER totalNumberOfFreeBytes = {{ 0, 0 }};
        if (!GetDiskFreeSpaceExW(GHC_NATIVEWP(p), &freeBytesAvailableToCaller, &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
            ec = detail::make_system_error();
            return {static_cast<uintmax_t>(-1), static_cast<uintmax_t>(-1), static_cast<uintmax_t>(-1)};
        }
        return {static_cast<uintmax_t>(totalNumberOfBytes.QuadPart), static_cast<uintmax_t>(totalNumberOfFreeBytes.QuadPart), static_cast<uintmax_t>(freeBytesAvailableToCaller.QuadPart)};
#else
        struct ::statvfs sfs;
        if (::statvfs(p.c_str(), &sfs) != 0) {
            ec = detail::make_system_error();
            return {static_cast<uintmax_t>(-1), static_cast<uintmax_t>(-1), static_cast<uintmax_t>(-1)};
        }
        return {static_cast<uintmax_t>(sfs.f_blocks) * static_cast<uintmax_t>(sfs.f_frsize),
                static_cast<uintmax_t>(sfs.f_bfree) * static_cast<uintmax_t>(sfs.f_frsize),
                static_cast<uintmax_t>(sfs.f_bavail) * static_cast<uintmax_t>(sfs.f_frsize)};
#endif
    }

    Result<FileStatus> status(const FilePath &p) {
        std::error_code ec;
        auto result = status(p, ec);
        if (result.type() == FileType::none) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), p, ec);
        }
        return result;
    }

    FileStatus status(const FilePath &p, std::error_code &ec) noexcept {
        return detail::status_ex(p, ec);
    }

    bool status_known(FileStatus s) noexcept {
        return s.type() != FileType::none;
    }

    Result<FileStatus> symlink_status(const FilePath &p) {
        std::error_code ec;
        auto result = symlink_status(p, ec);
        if (result.type() == FileType::none) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), p, ec);
        }
        return result;
    }

    FileStatus symlink_status(const FilePath &p, std::error_code &ec) noexcept {
        return detail::symlink_status_ex(p, ec);
    }


#ifdef TURBO_FS_WITH_EXCEPTIONS

    Result<FilePath> temp_directory_path() {
        std::error_code ec;
        FilePath result = temp_directory_path(ec);
        if (ec) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), ec);
        }
        return result;
    }

#endif

    FilePath temp_directory_path(std::error_code &ec) noexcept {
        ec.clear();
#ifdef OS_WIN
        wchar_t buffer[512];
        auto rc = GetTempPathW(511, buffer);
        if (!rc || rc > 511) {
            ec = detail::make_system_error();
            return FilePath{};
        }
        return FilePath(std::wstring(buffer));
#else
        static const char *temp_vars[] = {"TMPDIR", "TMP", "TEMP", "TEMPDIR", nullptr};
        const char *temp_path = nullptr;
        for (auto temp_name = temp_vars; *temp_name != nullptr; ++temp_name) {
            temp_path = std::getenv(*temp_name);
            if (temp_path) {
                return FilePath(temp_path);
            }
        }
        return FilePath{"/tmp"};
#endif
    }

    Result<FilePath> weakly_canonical(const FilePath &p) {
        std::error_code ec;
        auto result = weakly_canonical(p, ec);
        if (ec) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), p, ec);
        }
        return result;
    }


    FilePath weakly_canonical(const FilePath &p, std::error_code &ec) noexcept {
        FilePath result;
        ec.clear();
        bool scan = true;
        for (auto pe: p) {
            if (scan) {
                std::error_code tec;
                if (exists(result / pe, tec)) {
                    result /= pe;
                } else {
                    if (ec) {
                        return FilePath{};
                    }
                    scan = false;
                    if (!result.empty()) {
                        result = canonical(result, ec) / pe;
                        if (ec) {
                            break;
                        }
                    } else {
                        result /= pe;
                    }
                }
            } else {
                result /= pe;
            }
        }
        if (scan) {
            if (!result.empty()) {
                result = canonical(result, ec);
            }
        }
        return ec ? FilePath{} : result.lexically_normal();
    }

    //-----------------------------------------------------------------------------
    // [fs.class.FileStatus] class FileStatus
    // [fs.FileStatus.cons] constructors and destructor
    FileStatus::FileStatus() noexcept
            : FileStatus(FileType::none) {
    }

    FileStatus::FileStatus(FileType ft, FilePerms prms) noexcept
            : _type(ft), _perms(prms) {
    }

    FileStatus::FileStatus(const FileStatus &other) noexcept
            : _type(other._type), _perms(other._perms) {
    }

    FileStatus::FileStatus(FileStatus &&other) noexcept
            : _type(other._type), _perms(other._perms) {
    }

    FileStatus::~FileStatus() {}

    // assignments:
    FileStatus &FileStatus::operator=(const FileStatus &rhs) noexcept {
        _type = rhs._type;
        _perms = rhs._perms;
        return *this;
    }

    FileStatus &FileStatus::operator=(FileStatus &&rhs) noexcept {
        _type = rhs._type;
        _perms = rhs._perms;
        return *this;
    }

    // [fs.FileStatus.mods] modifiers
    void FileStatus::type(FileType ft) noexcept {
        _type = ft;
    }

    void FileStatus::permissions(FilePerms prms) noexcept {
        _perms = prms;
    }

    // [fs.FileStatus.obs] observers
    FileType FileStatus::type() const noexcept {
        return _type;
    }

    FilePerms FileStatus::permissions() const noexcept {
        return _perms;
    }


    Result<DirectoryEntry> DirectoryEntry::create(const FilePath &p) {
        std::error_code ec;
        DirectoryEntry ety(p, ec);
        if (ec) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), p, ec);
        }
        return ety;
    }


    DirectoryEntry::DirectoryEntry(const FilePath &p, std::error_code &ec)
            : _path(p), _file_size(static_cast<uintmax_t>(-1))
#ifndef OS_WIN
            , _hard_link_count(static_cast<uintmax_t>(-1))
#endif
            , _last_write_time(0) {
        refresh(ec);
    }

    DirectoryEntry::~DirectoryEntry() {}

    // assignments:
    // DirectoryEntry& DirectoryEntry::operator=(const DirectoryEntry&) = default;
    // DirectoryEntry& DirectoryEntry::operator=(DirectoryEntry&&) noexcept = default;

    // [fs.dir.entry.mods] DirectoryEntry modifiers

    Status DirectoryEntry::assign(const FilePath &p) {
        _path = p;
        return refresh();
    }


    void DirectoryEntry::assign(const FilePath &p, std::error_code &ec) {
        _path = p;
        refresh(ec);
    }


    Status DirectoryEntry::replace_filename(const FilePath &p) {
        _path.replace_filename(p);
        return refresh();
    }


    void DirectoryEntry::replace_filename(const FilePath &p, std::error_code &ec) {
        _path.replace_filename(p);
        refresh(ec);
    }


    Status DirectoryEntry::refresh() {
        std::error_code ec;
        refresh(ec);
        if (ec && (_status.type() == FileType::none || _symlink_status.type() != FileType::symlink)) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), _path, ec);
        }
        return turbo::OkStatus();
    }


    void DirectoryEntry::refresh(std::error_code &ec) noexcept {
#ifdef OS_WIN
        _status = detail::status_ex(_path, ec, &_symlink_status, &_file_size, nullptr, &_last_write_time);
#else
        _status = detail::status_ex(_path, ec, &_symlink_status, &_file_size, &_hard_link_count, &_last_write_time);
#endif
    }

// [fs.dir.entry.obs] DirectoryEntry observers
    const FilePath &DirectoryEntry::path() const noexcept {
        return _path;
    }

    DirectoryEntry::operator const FilePath &() const noexcept {
        return _path;
    }


    Result<FileType> DirectoryEntry::status_file_type() const {

        if (_status.type() != FileType::none) {
            return _status.type();
        }
        TURBO_MOVE_OR_RAISE(auto s, turbo::status(path()));
        return s.type();
    }


    FileType DirectoryEntry::status_file_type(std::error_code &ec) const noexcept {
        if (_status.type() != FileType::none) {
            ec.clear();
            return _status.type();
        }
        return turbo::status(path(), ec).type();
    }

    Result<bool> DirectoryEntry::exists() const {
        TURBO_MOVE_OR_RAISE(auto s, status_file_type());
        return s != FileType::not_found;
    }


    bool DirectoryEntry::exists(std::error_code &ec) const noexcept {
        return status_file_type(ec) != FileType::not_found;
    }


    Result<bool> DirectoryEntry::is_block_file() const {
        TURBO_MOVE_OR_RAISE(auto s, status_file_type());
        return s == FileType::block;
    }


    bool DirectoryEntry::is_block_file(std::error_code &ec) const noexcept {
        return status_file_type(ec) == FileType::block;
    }

    Result<bool> DirectoryEntry::is_character_file() const {
        TURBO_MOVE_OR_RAISE(auto s, status_file_type());
        return s == FileType::character;
    }


    bool DirectoryEntry::is_character_file(std::error_code &ec) const noexcept {
        return status_file_type(ec) == FileType::character;
    }

    Result<bool> DirectoryEntry::is_directory() const {
        TURBO_MOVE_OR_RAISE(auto s, status_file_type());
        return s == FileType::directory;
    }


    bool DirectoryEntry::is_directory(std::error_code &ec) const noexcept {
        return status_file_type(ec) == FileType::directory;
    }


    Result<bool> DirectoryEntry::is_fifo() const {
        TURBO_MOVE_OR_RAISE(auto s, status_file_type());
        return s == FileType::fifo;
    }


    bool DirectoryEntry::is_fifo(std::error_code &ec) const noexcept {
        return status_file_type(ec) == FileType::fifo;
    }


    Result<bool> DirectoryEntry::is_other() const {
        TURBO_MOVE_OR_RAISE(auto ft, status_file_type());
        TURBO_MOVE_OR_RAISE(auto sym, is_symlink());
        return ft != FileType::none && ft != FileType::not_found && ft != FileType::regular &&
               ft != FileType::directory && !sym;
    }


    bool DirectoryEntry::is_other(std::error_code &ec) const noexcept {
        auto ft = status_file_type(ec);
        bool other = ft != FileType::none && ft != FileType::not_found && ft != FileType::regular &&
                     ft != FileType::directory && !is_symlink(ec);
        return !ec && other;
    }

    Result<bool> DirectoryEntry::is_regular_file() const {
        TURBO_MOVE_OR_RAISE(auto ft, status_file_type());
        return ft == FileType::regular;
    }

    bool DirectoryEntry::is_regular_file(std::error_code &ec) const noexcept {
        return status_file_type(ec) == FileType::regular;
    }


    Result<bool> DirectoryEntry::is_socket() const {
        TURBO_MOVE_OR_RAISE(auto ft, status_file_type());
        return ft == FileType::socket;
    }


    bool DirectoryEntry::is_socket(std::error_code &ec) const noexcept {
        return status_file_type(ec) == FileType::socket;
    }


    Result<bool> DirectoryEntry::is_symlink() const {
        if (_symlink_status.type() != FileType::none) {
            return _symlink_status.type() == FileType::symlink;
        }
        TURBO_MOVE_OR_RAISE(auto ft, symlink_status());
        return turbo::is_symlink(ft);
    }


    bool DirectoryEntry::is_symlink(std::error_code &ec) const noexcept {
        if (_symlink_status.type() != FileType::none) {
            ec.clear();
            return _symlink_status.type() == FileType::symlink;
        }
        return turbo::is_symlink(symlink_status(ec));
    }


    Result<uintmax_t> DirectoryEntry::file_size() const {
        if (_file_size != static_cast<uintmax_t>(-1)) {
            return _file_size;
        }
        return turbo::file_size(path());
    }

    uintmax_t DirectoryEntry::file_size(std::error_code &ec) const noexcept {
        if (_file_size != static_cast<uintmax_t>(-1)) {
            ec.clear();
            return _file_size;
        }
        return turbo::file_size(path(), ec);
    }

#ifndef OS_WEB

    Result<uintmax_t> DirectoryEntry::hard_link_count() const {
#ifndef OS_WIN
        if (_hard_link_count != static_cast<uintmax_t>(-1)) {
            return _hard_link_count;
        }
#endif
        return turbo::hard_link_count(path());
    }

    uintmax_t DirectoryEntry::hard_link_count(std::error_code &ec) const noexcept {
#ifndef OS_WIN
        if (_hard_link_count != static_cast<uintmax_t>(-1)) {
            ec.clear();
            return _hard_link_count;
        }
#endif
        return turbo::hard_link_count(path(), ec);
    }

#endif

    Result<Time> DirectoryEntry::last_write_time() const {
        if (_last_write_time != 0) {
            return Time::from_time_t(_last_write_time);
        }
        return turbo::last_write_time(path());
    }


    Time DirectoryEntry::last_write_time(std::error_code &ec) const noexcept {
        if (_last_write_time != 0) {
            ec.clear();
            return Time::from_time_t(_last_write_time);
        }
        return turbo::last_write_time(path(), ec);
    }


    Result<FileStatus> DirectoryEntry::status() const {
        if (_status.type() != FileType::none && _status.permissions() != FilePerms::unknown) {
            return _status;
        }
        return turbo::status(path());
    }


    FileStatus DirectoryEntry::status(std::error_code &ec) const noexcept {
        if (_status.type() != FileType::none && _status.permissions() != FilePerms::unknown) {
            ec.clear();
            return _status;
        }
        return turbo::status(path(), ec);
    }


    Result<FileStatus> DirectoryEntry::symlink_status() const {
        if (_symlink_status.type() != FileType::none && _symlink_status.permissions() != FilePerms::unknown) {
            return _symlink_status;
        }
        return turbo::symlink_status(path());
    }


    FileStatus DirectoryEntry::symlink_status(std::error_code &ec) const noexcept {
        if (_symlink_status.type() != FileType::none && _symlink_status.permissions() != FilePerms::unknown) {
            ec.clear();
            return _symlink_status;
        }
        return turbo::symlink_status(path(), ec);
    }


    bool DirectoryEntry::operator<(const DirectoryEntry &rhs) const noexcept {
        return _path < rhs._path;
    }

    bool DirectoryEntry::operator==(const DirectoryEntry &rhs) const noexcept {
        return _path == rhs._path;
    }

    bool DirectoryEntry::operator!=(const DirectoryEntry &rhs) const noexcept {
        return _path != rhs._path;
    }

    bool DirectoryEntry::operator<=(const DirectoryEntry &rhs) const noexcept {
        return _path <= rhs._path;
    }

    bool DirectoryEntry::operator>(const DirectoryEntry &rhs) const noexcept {
        return _path > rhs._path;
    }

    bool DirectoryEntry::operator>=(const DirectoryEntry &rhs) const noexcept {
        return _path >= rhs._path;
    }


    //-----------------------------------------------------------------------------
    // [fs.class.DirectoryIterator] class DirectoryIterator

#ifdef OS_WIN
    class DirectoryIterator::impl {
    public:
        impl(const FilePath& p, DirectoryOptions options)
            : _base(p)
            , _options(options)
            , _dirHandle(INVALID_HANDLE_VALUE)
        {
            if (!_base.empty()) {
                ZeroMemory(&_findData, sizeof(WIN32_FIND_DATAW));
                if ((_dirHandle = FindFirstFileW(GHC_NATIVEWP((_base / "*")), &_findData)) != INVALID_HANDLE_VALUE) {
                    if (std::wstring(_findData.cFileName) == L"." || std::wstring(_findData.cFileName) == L"..") {
                        increment(_ec);
                    }
                    else {
                        _dir_entry._path = _base / std::wstring(_findData.cFileName);
                        copyToDirEntry(_ec);
                    }
                }
                else {
                    auto error = ::GetLastError();
                    _base = turbo::FilePath();
                    if (error != ERROR_ACCESS_DENIED || (options & DirectoryOptions::skip_permission_denied) == DirectoryOptions::none) {
                        _ec = detail::make_system_error();
                    }
                }
            }
        }
        impl(const impl& other) = delete;
        ~impl()
        {
            if (_dirHandle != INVALID_HANDLE_VALUE) {
                FindClose(_dirHandle);
                _dirHandle = INVALID_HANDLE_VALUE;
            }
        }
        void increment(std::error_code& ec)
        {
            if (_dirHandle != INVALID_HANDLE_VALUE) {
                do {
                    if (FindNextFileW(_dirHandle, &_findData)) {
                        _dir_entry._path = _base;
#ifdef GHC_USE_WCHAR_T
                        _dir_entry._path.append_name(_findData.cFileName);
#else
                        _dir_entry._path.append_name(detail::toUtf8(_findData.cFileName).c_str());
#endif
                        copyToDirEntry(ec);
                    }
                    else {
                        auto err = ::GetLastError();
                        if (err != ERROR_NO_MORE_FILES) {
                            _ec = ec = detail::make_system_error(err);
                        }
                        FindClose(_dirHandle);
                        _dirHandle = INVALID_HANDLE_VALUE;
                        _dir_entry._path.clear();
                        break;
                    }
                } while (std::wstring(_findData.cFileName) == L"." || std::wstring(_findData.cFileName) == L"..");
            }
            else {
                ec = _ec;
            }
        }
        void copyToDirEntry(std::error_code& ec)
        {
            if (_findData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
                _dir_entry._status = detail::status_ex(_dir_entry._path, ec, &_dir_entry._symlink_status, &_dir_entry._file_size, nullptr, &_dir_entry._last_write_time);
            }
            else {
                _dir_entry._status = detail::status_from_INFO(_dir_entry._path, &_findData, ec, &_dir_entry._file_size, &_dir_entry._last_write_time);
                _dir_entry._symlink_status = _dir_entry._status;
            }
            if (ec) {
                if (_dir_entry._status.type() != FileType::none && _dir_entry._symlink_status.type() != FileType::none) {
                    ec.clear();
                }
                else {
                    _dir_entry._file_size = static_cast<uintmax_t>(-1);
                    _dir_entry._last_write_time = 0;
                }
            }
        }
        FilePath _base;
        DirectoryOptions _options;
        WIN32_FIND_DATAW _findData;
        HANDLE _dirHandle;
        DirectoryEntry _dir_entry;
        std::error_code _ec;
    };
#else

    // POSIX implementation
    class DirectoryIterator::impl {
    public:
        impl(const FilePath &path, DirectoryOptions options)
                : _base(path), _options(options), _dir(nullptr), _entry(nullptr) {
            if (!path.empty()) {
                do { _dir = ::opendir(path.native().c_str()); } while (errno == EINTR && !_dir);
                if (!_dir) {
                    auto error = errno;
                    _base = FilePath();
                    if ((error != EACCES && error != EPERM) ||
                        (options & DirectoryOptions::skip_permission_denied) == DirectoryOptions::none) {
                        _ec = detail::make_system_error();
                    }
                } else {
                    increment(_ec);
                }
            }
        }

        impl(const impl &other) = delete;

        ~impl() {
            if (_dir) {
                ::closedir(_dir);
            }
        }

        void increment(std::error_code &ec) {
            if (_dir) {
                bool skip;
                do {
                    skip = false;
                    errno = 0;
                    do { _entry = ::readdir(_dir); } while (errno == EINTR && !_entry);
                    if (_entry) {
                        _dir_entry._path = _base;
                        _dir_entry._path.append_name(_entry->d_name);
                        copyToDirEntry();
                        if (ec && (ec.value() == EACCES || ec.value() == EPERM) &&
                            (_options & DirectoryOptions::skip_permission_denied) ==
                            DirectoryOptions::skip_permission_denied) {
                            ec.clear();
                            skip = true;
                        }
                    } else {
                        ::closedir(_dir);
                        _dir = nullptr;
                        _dir_entry._path.clear();
                        if (errno && errno != EINTR) {
                            ec = detail::make_system_error();
                        }
                        break;
                    }
                } while (skip || std::strcmp(_entry->d_name, ".") == 0 || std::strcmp(_entry->d_name, "..") == 0);
            }
        }

        void copyToDirEntry() {
            _dir_entry._symlink_status.permissions(FilePerms::unknown);
            auto ft = detail::file_type_from_dirent(*_entry);
            _dir_entry._symlink_status.type(ft);
            if (ft != FileType::symlink) {
                _dir_entry._status = _dir_entry._symlink_status;
            } else {
                _dir_entry._status.type(FileType::none);
                _dir_entry._status.permissions(FilePerms::unknown);
            }
            _dir_entry._file_size = static_cast<uintmax_t>(-1);
            _dir_entry._hard_link_count = static_cast<uintmax_t>(-1);
            _dir_entry._last_write_time = 0;
        }

        FilePath _base;
        DirectoryOptions _options;
        DIR *_dir;
        struct ::dirent *_entry;
        DirectoryEntry _dir_entry;
        std::error_code _ec;
    };

#endif

    // [fs.dir.itr.members] member functions
    DirectoryIterator::DirectoryIterator() noexcept
            : _impl(new impl(FilePath(), DirectoryOptions::none)) {
    }


    Result<DirectoryIterator> DirectoryIterator::create(const FilePath &p, DirectoryOptions options) {
        std::error_code ec;
        DirectoryIterator itr(p, options, ec);
        if (ec) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), p, ec);;
        }
        return itr;
    }

    DirectoryIterator::DirectoryIterator(const FilePath &p, std::error_code &ec) noexcept
            : _impl(new impl(p, DirectoryOptions::none)) {
        if (_impl->_ec) {
            ec = _impl->_ec;
        }
    }

    DirectoryIterator::DirectoryIterator(const FilePath &p, DirectoryOptions options, std::error_code &ec) noexcept
            : _impl(new impl(p, options)) {
        if (_impl->_ec) {
            ec = _impl->_ec;
        }
    }

    DirectoryIterator::DirectoryIterator(const DirectoryIterator &rhs)
            : _impl(rhs._impl) {
    }

    DirectoryIterator::DirectoryIterator(DirectoryIterator &&rhs) noexcept
            : _impl(std::move(rhs._impl)) {
    }


    DirectoryIterator::~DirectoryIterator() {}

    DirectoryIterator &DirectoryIterator::operator=(const DirectoryIterator &rhs) {
        _impl = rhs._impl;
        return *this;
    }

    DirectoryIterator &DirectoryIterator::operator=(DirectoryIterator &&rhs) noexcept {
        _impl = std::move(rhs._impl);
        return *this;
    }

    const DirectoryEntry &DirectoryIterator::operator*() const {
        return _impl->_dir_entry;
    }

    const DirectoryEntry *DirectoryIterator::operator->() const {
        return &_impl->_dir_entry;
    }

#ifdef TURBO_FS_WITH_EXCEPTIONS

    DirectoryIterator &DirectoryIterator::operator++() {
        std::error_code ec;
        _impl->increment(ec);
        if (ec) {
            throw filesystem_error(detail::systemErrorText(ec.value()), _impl->_dir_entry._path, ec);
        }
        return *this;
    }

#endif

    DirectoryIterator &DirectoryIterator::increment(std::error_code &ec) noexcept {
        _impl->increment(ec);
        return *this;
    }

    bool DirectoryIterator::operator==(const DirectoryIterator &rhs) const {
        return _impl->_dir_entry._path == rhs._impl->_dir_entry._path;
    }

    bool DirectoryIterator::operator!=(const DirectoryIterator &rhs) const {
        return _impl->_dir_entry._path != rhs._impl->_dir_entry._path;
    }

    // [fs.dir.itr.nonmembers] DirectoryIterator non-member functions

    DirectoryIterator begin(DirectoryIterator iter) noexcept {
        return iter;
    }

    DirectoryIterator end(const DirectoryIterator &) noexcept {
        return DirectoryIterator();
    }


    //-----------------------------------------------------------------------------
    // [fs.class.rec.dir.itr] class RecursiveDirectoryIterator

    RecursiveDirectoryIterator::RecursiveDirectoryIterator() noexcept
            : _impl(new RecursiveDirectoryIteratorImpl(DirectoryOptions::none, true)) {
        _impl->_dir_iter_stack.push(DirectoryIterator());
    }

    Result<RecursiveDirectoryIterator> RecursiveDirectoryIterator::create(const FilePath &p, DirectoryOptions options) {
        std::error_code ec;
        RecursiveDirectoryIterator itr(p, options, ec);
        if (ec) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()), p, ec);
        }
        return itr;
    }

    RecursiveDirectoryIterator::RecursiveDirectoryIterator(const FilePath &p, DirectoryOptions options,
                                                           std::error_code &ec) noexcept
            : _impl(new RecursiveDirectoryIteratorImpl(options, true)) {
        _impl->_dir_iter_stack.push(DirectoryIterator(p, options, ec));
    }

    RecursiveDirectoryIterator::RecursiveDirectoryIterator(const FilePath &p, std::error_code &ec) noexcept
            : _impl(new RecursiveDirectoryIteratorImpl(DirectoryOptions::none, true)) {
        _impl->_dir_iter_stack.push(DirectoryIterator(p, ec));
    }

    RecursiveDirectoryIterator::RecursiveDirectoryIterator(const RecursiveDirectoryIterator &rhs)
            : _impl(rhs._impl) {
    }

    RecursiveDirectoryIterator::RecursiveDirectoryIterator(RecursiveDirectoryIterator &&rhs) noexcept
            : _impl(std::move(rhs._impl)) {
    }

    RecursiveDirectoryIterator::~RecursiveDirectoryIterator() {}

    // [fs.rec.dir.itr.members] observers
    DirectoryOptions RecursiveDirectoryIterator::options() const {
        return _impl->_options;
    }

    int RecursiveDirectoryIterator::depth() const {
        return static_cast<int>(_impl->_dir_iter_stack.size() - 1);
    }


    bool RecursiveDirectoryIterator::recursion_pending() const {
        return _impl->_recursion_pending;
    }

    const DirectoryEntry &RecursiveDirectoryIterator::operator*() const {
        return *(_impl->_dir_iter_stack.top());
    }

    const DirectoryEntry *RecursiveDirectoryIterator::operator->() const {
        return &(*(_impl->_dir_iter_stack.top()));
    }

    // [fs.rec.dir.itr.members] modifiers RecursiveDirectoryIterator&
    RecursiveDirectoryIterator &RecursiveDirectoryIterator::operator=(const RecursiveDirectoryIterator &rhs) {
        _impl = rhs._impl;
        return *this;
    }


    RecursiveDirectoryIterator &RecursiveDirectoryIterator::operator=(RecursiveDirectoryIterator &&rhs) noexcept {
        _impl = std::move(rhs._impl);
        return *this;
    }

#ifdef TURBO_FS_WITH_EXCEPTIONS

    RecursiveDirectoryIterator &RecursiveDirectoryIterator::operator++() {
        std::error_code ec;
        increment(ec);
        if (ec) {
            throw filesystem_error(detail::systemErrorText(ec.value()),
                                   _impl->_dir_iter_stack.empty() ? FilePath() : _impl->_dir_iter_stack.top()->path(),
                                   ec);
        }
        return *this;
    }

#endif

    RecursiveDirectoryIterator &RecursiveDirectoryIterator::increment(std::error_code &ec) noexcept {
        bool isSymLink = (*this)->is_symlink(ec);
        bool isDir = !ec && (*this)->is_directory(ec);
        if (isSymLink && detail::is_not_found_error(ec)) {
            ec.clear();
        }
        if (!ec) {
            if (recursion_pending() && isDir &&
                (!isSymLink || (options() & DirectoryOptions::follow_directory_symlink) != DirectoryOptions::none)) {
                _impl->_dir_iter_stack.push(DirectoryIterator((*this)->path(), _impl->_options, ec));
            } else {
                _impl->_dir_iter_stack.top().increment(ec);
            }
            if (!ec) {
                while (depth() && _impl->_dir_iter_stack.top() == DirectoryIterator()) {
                    _impl->_dir_iter_stack.pop();
                    _impl->_dir_iter_stack.top().increment(ec);
                }
            } else if (!_impl->_dir_iter_stack.empty()) {
                _impl->_dir_iter_stack.pop();
            }
            _impl->_recursion_pending = true;
        }
        return *this;
    }

    Status RecursiveDirectoryIterator::pop() {
        std::error_code ec;
        pop(ec);
        if (ec) {
            return fs_detail::error_code_to_status(detail::systemErrorText(ec.value()),
                                                   _impl->_dir_iter_stack.empty() ? FilePath()
                                                                                  : _impl->_dir_iter_stack.top()->path(),
                                                   ec);
        }
        return turbo::OkStatus();
    }

    void RecursiveDirectoryIterator::pop(std::error_code &ec) {
        if (depth() == 0) {
            *this = RecursiveDirectoryIterator();
        } else {
            do {
                _impl->_dir_iter_stack.pop();
                _impl->_dir_iter_stack.top().increment(ec);
            } while (depth() && _impl->_dir_iter_stack.top() == DirectoryIterator());
        }
    }


    void RecursiveDirectoryIterator::disable_recursion_pending() {
        _impl->_recursion_pending = false;
    }

    // other members as required by [input.iterators]
    bool RecursiveDirectoryIterator::operator==(const RecursiveDirectoryIterator &rhs) const {
        return _impl->_dir_iter_stack.top() == rhs._impl->_dir_iter_stack.top();
    }

    bool RecursiveDirectoryIterator::operator!=(const RecursiveDirectoryIterator &rhs) const {
        return _impl->_dir_iter_stack.top() != rhs._impl->_dir_iter_stack.top();
    }

    // [fs.rec.dir.itr.nonmembers] DirectoryIterator non-member functions
    RecursiveDirectoryIterator begin(RecursiveDirectoryIterator iter) noexcept {
        return iter;
    }

    RecursiveDirectoryIterator end(const RecursiveDirectoryIterator &) noexcept {
        return RecursiveDirectoryIterator();
    }
}  // namespace turbo

