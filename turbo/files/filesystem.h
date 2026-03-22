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

#include <turbo/base/macros.h>
#include <turbo/utility/status.h>
// #define BSD manifest constant only in
// sys/param.h
#ifndef _WIN32

#include <sys/param.h>

#endif

#ifdef OS_WIN
#include <windows.h>
// additional includes
#include <shellapi.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <wchar.h>
#include <winioctl.h>
#else

#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#if  defined(__ANDROID__)
#include <android/api-level.h>
#if __ANDROID_API__ < 12
#include <sys/syscall.h>
#endif
#include <sys/vfs.h>
#define statvfs statfs
#else

#include <sys/statvfs.h>

#endif
#if defined(__CYGWIN__)
#include <strings.h>
#endif
#if !defined(__ANDROID__) || __ANDROID_API__ >= 26

#include <langinfo.h>

#endif
#endif
#ifdef OS_MACOSX
#include <Availability.h>
#endif

#include <algorithm>
#include <cctype>
#include <clocale>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <memory>
#include <stack>
#include <stdexcept>
#include <string>
#include <system_error>
#include <type_traits>
#include <utility>
#include <vector>
#include <string_view>

#if !defined(OS_WIN) && !defined(PATH_MAX)
#define PATH_MAX 4096
#endif

#if !defined(TURBO_FS_WITH_EXCEPTIONS) && (defined(__EXCEPTIONS) || defined(__cpp_exceptions) || defined(_CPPUNWIND))
#define TURBO_FS_WITH_EXCEPTIONS
#endif

namespace turbo {

    using std::basic_string_view;

    template<typename char_type>
    class PathHelperBase {
    public:
        using value_type = char_type;
#ifdef OS_WIN
        static constexpr value_type preferred_separator = '\\';
#else
        static constexpr value_type preferred_separator = '/';
#endif
    };

#if __cplusplus < 201703L
    template <typename char_type>
    constexpr char_type PathHelperBase<char_type>::preferred_separator;
#endif

#ifdef OS_WIN
    class path;
    namespace detail {
    bool has_executable_extension(const path& p);
    }
#endif

    // [fs.class.path] class path
    class TURBO_EXPORT FilePath
#if defined(OS_WIN)
#define GHC_USE_WCHAR_T
#define GHC_NATIVEWP(p) p.c_str()
#define GHC_PLATFORM_LITERAL(str) L##str
        : private PathHelperBase<std::wstring::value_type>
    {
    public:
        using PathHelperBase<std::wstring::value_type>::value_type;
#else
#define GHC_NATIVEWP(p) p.wstring().c_str()
#define GHC_PLATFORM_LITERAL(str) str
            : private PathHelperBase<std::string::value_type> {
    public:
        using PathHelperBase<std::string::value_type>::value_type;
#endif
        using string_type = std::basic_string<value_type>;
        using string_view_type = std::basic_string_view<value_type>;
        using PathHelperBase<value_type>::preferred_separator;

        // [fs.enum.path.format] enumeration format
        /// The path format in which the constructor argument is given.
        enum format {
            generic_format,  ///< The generic format, internally used by
            ///< ghc::filesystem::path with slashes
            native_format,   ///< The format native to the current platform this code
            ///< is build for
            auto_format,     ///< Try to auto-detect the format, fallback to native
        };

        template<class T>
        struct _is_basic_string : std::false_type {
        };
        template<class CharT, class Traits, class Alloc>
        struct _is_basic_string<std::basic_string<CharT, Traits, Alloc>> : std::true_type {
        };
        template<class CharT>
        struct _is_basic_string<std::basic_string<CharT, std::char_traits<CharT>, std::allocator<CharT>>>
                : std::true_type {
        };
        template<class CharT, class Traits>
        struct _is_basic_string<basic_string_view<CharT, Traits>> : std::true_type {
        };
        template<class CharT>
        struct _is_basic_string<basic_string_view<CharT, std::char_traits<CharT>>> : std::true_type {
        };

        template<typename T1, typename T2 = void>
        using path_type = typename std::enable_if<!std::is_same<FilePath, T1>::value, FilePath>::type;
        template<typename T>
#if defined(__cpp_lib_char8_t)
            using path_from_string =
                typename std::enable_if<_is_basic_string<T>::value || std::is_same<char const*, typename std::decay<T>::type>::value || std::is_same<char*, typename std::decay<T>::type>::value || std::is_same<char8_t const*, typename std::decay<T>::type>::value ||
                                            std::is_same<char8_t*, typename std::decay<T>::type>::value || std::is_same<char16_t const*, typename std::decay<T>::type>::value || std::is_same<char16_t*, typename std::decay<T>::type>::value ||
                                            std::is_same<char32_t const*, typename std::decay<T>::type>::value || std::is_same<char32_t*, typename std::decay<T>::type>::value || std::is_same<wchar_t const*, typename std::decay<T>::type>::value ||
                                            std::is_same<wchar_t*, typename std::decay<T>::type>::value,
                                        path>::type;
            template <typename T>
            using path_type_EcharT = typename std::enable_if<std::is_same<T, char>::value || std::is_same<T, char8_t>::value || std::is_same<T, char16_t>::value || std::is_same<T, char32_t>::value || std::is_same<T, wchar_t>::value, path>::type;
#else
        using path_from_string =
                typename std::enable_if<_is_basic_string<T>::value ||
                                        std::is_same<char const *, typename std::decay<T>::type>::value ||
                                        std::is_same<char *, typename std::decay<T>::type>::value ||
                                        std::is_same<char16_t const *, typename std::decay<T>::type>::value ||
                                        std::is_same<char16_t *, typename std::decay<T>::type>::value ||
                                        std::is_same<char32_t const *, typename std::decay<T>::type>::value ||
                                        std::is_same<char32_t *, typename std::decay<T>::type>::value ||
                                        std::is_same<wchar_t const *, typename std::decay<T>::type>::value ||
                                        std::is_same<wchar_t *, typename std::decay<T>::type>::value,
                        FilePath>::type;
        template<typename T>
        using path_type_EcharT = typename std::enable_if<
                std::is_same<T, char>::value || std::is_same<T, char16_t>::value ||
                std::is_same<T, char32_t>::value || std::is_same<T, wchar_t>::value, FilePath>::type;
#endif

        // [fs.path.construct] constructors and destructor
        FilePath() noexcept;

        FilePath(const FilePath &p);

        FilePath(FilePath &&p) noexcept;

        FilePath(string_type &&source, format fmt = auto_format);

        template<class Source, typename = path_from_string<Source>>
        FilePath(const Source &source, format fmt = auto_format);

        template<class InputIterator>
        FilePath(InputIterator first, InputIterator last, format fmt = auto_format);


        template<class Source, typename = path_from_string<Source>>
        static Result<FilePath> create(const Source &source, const std::locale &loc, format fmt = auto_format);

        template<class InputIterator>
        static Result<FilePath>
        create(InputIterator first, InputIterator last, const std::locale &loc, format fmt = auto_format);


        ~FilePath() = default;

        // [fs.path.assign] assignments
        FilePath &operator=(const FilePath &p);

        FilePath &operator=(FilePath &&p) noexcept;

        FilePath &operator=(string_type &&source);

        FilePath &assign(string_type &&source);

        template<class Source>
        FilePath &operator=(const Source &source);

        template<class Source>
        FilePath &assign(const Source &source);

        template<class InputIterator>
        FilePath &assign(InputIterator first, InputIterator last);

        // [fs.path.append] appends
        FilePath &operator/=(const FilePath &p);

        template<class Source>
        FilePath &operator/=(const Source &source);

        template<class Source>
        FilePath &append(const Source &source);

        template<class InputIterator>
        FilePath &append(InputIterator first, InputIterator last);

        // [fs.path.concat] concatenation
        FilePath &operator+=(const FilePath &x);

        FilePath &operator+=(const string_type &x);

        FilePath &operator+=(basic_string_view<value_type> x);

        FilePath &operator+=(const value_type *x);

        FilePath &operator+=(value_type x);

        template<class Source>
        path_from_string<Source> &operator+=(const Source &x);

        template<class EcharT>
        path_type_EcharT<EcharT> &operator+=(EcharT x);

        template<class Source>
        FilePath &concat(const Source &x);

        template<class InputIterator>
        FilePath &concat(InputIterator first, InputIterator last);

        // [fs.path.modifiers] modifiers
        void clear() noexcept;

        FilePath &make_preferred();

        FilePath &remove_filename();

        FilePath &replace_filename(const FilePath &replacement);

        FilePath &replace_extension(const FilePath &replacement = FilePath());

        void swap(FilePath &rhs) noexcept;

        // [fs.path.native.obs] native format observers
        const string_type &native() const noexcept;

        const value_type *c_str() const noexcept;

        operator string_type() const;

        template<class EcharT, class traits = std::char_traits<EcharT>, class Allocator = std::allocator<EcharT>>
        std::basic_string<EcharT, traits, Allocator> string(const Allocator &a = Allocator()) const;

        std::string string() const;

        std::wstring wstring() const;

#if defined(__cpp_lib_char8_t)
        std::u8string u8string() const;
#else

        std::string u8string() const;

#endif

        std::u16string u16string() const;

        std::u32string u32string() const;

        // [fs.path.generic.obs] generic format observers
        template<class EcharT, class traits = std::char_traits<EcharT>, class Allocator = std::allocator<EcharT>>
        std::basic_string<EcharT, traits, Allocator> generic_string(const Allocator &a = Allocator()) const;

        std::string generic_string() const;

        std::wstring generic_wstring() const;

#if defined(__cpp_lib_char8_t)
        std::u8string generic_u8string() const;
#else

        std::string generic_u8string() const;

#endif

        std::u16string generic_u16string() const;

        std::u32string generic_u32string() const;

        // [fs.path.compare] compare
        int compare(const FilePath &p) const noexcept;

        int compare(const string_type &s) const;

        int compare(basic_string_view<value_type> s) const;

        int compare(const value_type *s) const;

        // [fs.path.decompose] decomposition
        FilePath root_name() const;

        FilePath root_directory() const;

        FilePath root_path() const;

        FilePath relative_path() const;

        FilePath parent_path() const;

        FilePath filename() const;

        FilePath stem() const;

        FilePath extension() const;

        // [fs.path.query] query
        bool empty() const noexcept;

        bool has_root_name() const;

        bool has_root_directory() const;

        bool has_root_path() const;

        bool has_relative_path() const;

        bool has_parent_path() const;

        bool has_filename() const;

        bool has_stem() const;

        bool has_extension() const;

        bool is_absolute() const;

        bool is_relative() const;

        // [fs.path.gen] generation
        FilePath lexically_normal() const;

        FilePath lexically_relative(const FilePath &base) const;

        FilePath lexically_proximate(const FilePath &base) const;

        // [fs.path.itr] iterators
        class iterator;

        using const_iterator = iterator;

        iterator begin() const;

        iterator end() const;

        static turbo::Result<FilePath> from_string(string_view_type str);

    private:
        using impl_value_type = value_type;
        using impl_string_type = std::basic_string<impl_value_type>;

        friend class DirectoryIterator;

        void append_name(const value_type *name);

        static constexpr impl_value_type generic_separator = '/';

        template<typename InputIterator>
        class input_iterator_range {
        public:
            typedef InputIterator iterator;
            typedef InputIterator const_iterator;
            typedef typename InputIterator::difference_type difference_type;

            input_iterator_range(const InputIterator &first, const InputIterator &last)
                    : _first(first), _last(last) {
            }

            InputIterator begin() const { return _first; }

            InputIterator end() const { return _last; }

        private:
            InputIterator _first;
            InputIterator _last;
        };

        friend void swap(FilePath &lhs, FilePath &rhs) noexcept;

        friend size_t hash_value(const FilePath &p) noexcept;

        friend FilePath canonical(const FilePath &p, std::error_code &ec);

        friend bool create_directories(const FilePath &p, std::error_code &ec) noexcept;

        string_type::size_type root_name_length() const noexcept;

        void postprocess_path_with_format(format fmt);

        void check_long_path();

        impl_string_type _path;
#ifdef OS_WIN
        void handle_prefixes();
        friend bool detail::has_executable_extension(const path& p);
        string_type::size_type _prefixLength{0};
#else
        static const string_type::size_type _prefixLength{0};
#endif
    };

    // [fs.path.nonmember] path non-member functions
    TURBO_EXPORT void swap(FilePath &lhs, FilePath &rhs) noexcept;

    TURBO_EXPORT size_t hash_value(const FilePath &p) noexcept;

    TURBO_EXPORT bool operator==(const FilePath &lhs, const FilePath &rhs) noexcept;

    TURBO_EXPORT bool operator!=(const FilePath &lhs, const FilePath &rhs) noexcept;

    TURBO_EXPORT bool operator<(const FilePath &lhs, const FilePath &rhs) noexcept;

    TURBO_EXPORT bool operator<=(const FilePath &lhs, const FilePath &rhs) noexcept;

    TURBO_EXPORT bool operator>(const FilePath &lhs, const FilePath &rhs) noexcept;

    TURBO_EXPORT bool operator>=(const FilePath &lhs, const FilePath &rhs) noexcept;

    TURBO_EXPORT FilePath operator/(const FilePath &lhs, const FilePath &rhs);

    // [fs.path.io] path inserter and extractor
    template<class charT, class traits>
    std::basic_ostream<charT, traits> &operator<<(std::basic_ostream<charT, traits> &os, const FilePath &p);

    template<class charT, class traits>
    std::basic_istream<charT, traits> &operator>>(std::basic_istream<charT, traits> &is, FilePath &p);

    // [pfs.path.factory] path factory functions
    template<class Source, typename = FilePath::path_from_string<Source>>
#if defined(__cpp_lib_char8_t)
    [[deprecated("use ghc::filesystem::path::path() with std::u8string instead")]]
#endif
    FilePath u8path(const Source &source);

    template<class InputIterator>
#if defined(__cpp_lib_char8_t)
    [[deprecated("use ghc::filesystem::path::path() with std::u8string instead")]]
#endif
    FilePath u8path(InputIterator first, InputIterator last);

    // [fs.class.filesystem_error] class filesystem_error
    class TURBO_EXPORT filesystem_error : public std::system_error {
    public:
        filesystem_error(const std::string &what_arg, std::error_code ec);

        filesystem_error(const std::string &what_arg, const FilePath &p1, std::error_code ec);

        filesystem_error(const std::string &what_arg, const FilePath &p1, const FilePath &p2, std::error_code ec);

        const FilePath &path1() const noexcept;

        const FilePath &path2() const noexcept;

        const char *what() const noexcept override;

    private:
        std::string _what_arg;
        std::error_code _ec;
        FilePath _p1, _p2;
    };

    class TURBO_EXPORT FilePath::iterator {
    public:
        using value_type = const FilePath;
        using difference_type = std::ptrdiff_t;
        using pointer = const FilePath *;
        using reference = const FilePath &;
        using iterator_category = std::bidirectional_iterator_tag;

        iterator();

        iterator(const FilePath &p, const impl_string_type::const_iterator &pos);

        iterator &operator++();

        iterator operator++(int);

        iterator &operator--();

        iterator operator--(int);

        bool operator==(const iterator &other) const;

        bool operator!=(const iterator &other) const;

        reference operator*() const;

        pointer operator->() const;

    private:
        friend class FilePath;

        impl_string_type::const_iterator increment(const impl_string_type::const_iterator &pos) const;

        impl_string_type::const_iterator decrement(const impl_string_type::const_iterator &pos) const;

        void updateCurrent();

        impl_string_type::const_iterator _first;
        impl_string_type::const_iterator _last;
        impl_string_type::const_iterator _prefix;
        impl_string_type::const_iterator _root;
        impl_string_type::const_iterator _iter;
        FilePath _current;
    };

    struct FileSpaceInfo {
        uintmax_t capacity;
        uintmax_t free;
        uintmax_t available;
    };

    // [fs.enum] enumerations
    // [fs.enum.FileType]
    enum class FileType {
        none,
        not_found,
        regular,
        directory,
        symlink,
        block,
        character,
        fifo,
        socket,
        unknown,
    };

    // [fs.enum.FilePerms]
    enum class FilePerms : uint16_t {
        none = 0,

        owner_read = 0400,
        owner_write = 0200,
        owner_exec = 0100,
        owner_all = 0700,

        group_read = 040,
        group_write = 020,
        group_exec = 010,
        group_all = 070,

        others_read = 04,
        others_write = 02,
        others_exec = 01,
        others_all = 07,

        all = 0777,
        set_uid = 04000,
        set_gid = 02000,
        sticky_bit = 01000,

        mask = 07777,
        unknown = 0xffff
    };

    // [fs.enum.perm.opts]
    enum class FilePermOptions : uint16_t {
        replace = 3,
        add = 1,
        remove = 2,
        nofollow = 4,
    };

    // [fs.enum.copy.opts]
    enum class FileCopyOptions : uint16_t {
        none = 0,

        skip_existing = 1,
        overwrite_existing = 2,
        update_existing = 4,

        recursive = 8,

        copy_symlinks = 0x10,
        skip_symlinks = 0x20,

        directories_only = 0x40,
        create_symlinks = 0x80,
#ifndef OS_WEB
        create_hard_links = 0x100
#endif
    };

    // [fs.enum.dir.opts]
    enum class DirectoryOptions : uint16_t {
        none = 0,
        follow_directory_symlink = 1,
        skip_permission_denied = 2,
    };

    // [fs.class.FileStatus] class FileStatus
    class TURBO_EXPORT FileStatus {
    public:
        // [fs.FileStatus.cons] constructors and destructor
        FileStatus() noexcept;

        explicit FileStatus(FileType ft, FilePerms prms = FilePerms::unknown) noexcept;

        FileStatus(const FileStatus &) noexcept;

        FileStatus(FileStatus &&) noexcept;

        ~FileStatus();

        // assignments:
        FileStatus &operator=(const FileStatus &) noexcept;

        FileStatus &operator=(FileStatus &&) noexcept;

        // [fs.FileStatus.mods] modifiers
        void type(FileType ft) noexcept;

        void permissions(FilePerms prms) noexcept;

        // [fs.FileStatus.obs] observers
        FileType type() const noexcept;

        FilePerms permissions() const noexcept;

        friend bool operator==(const FileStatus &lhs, const FileStatus &rhs) noexcept {
            return lhs.type() == rhs.type() && lhs.permissions() == rhs.permissions();
        }

    private:
        FileType _type;
        FilePerms _perms;
    };

    // [fs.class.DirectoryEntry] Class DirectoryEntry
    class TURBO_EXPORT DirectoryEntry {
    public:
        // [fs.dir.entry.cons] constructors and destructor
        DirectoryEntry() noexcept = default;

        DirectoryEntry(const DirectoryEntry &) = default;

        DirectoryEntry(DirectoryEntry &&) noexcept = default;

        static Result<DirectoryEntry> create(const FilePath &p);

        DirectoryEntry(const FilePath &p, std::error_code &ec);

        ~DirectoryEntry();

        // assignments:
        DirectoryEntry &operator=(const DirectoryEntry &) = default;

        DirectoryEntry &operator=(DirectoryEntry &&) noexcept = default;

        // [fs.dir.entry.mods] modifiers

        Status assign(const FilePath &p);

        Status replace_filename(const FilePath &p);

        Status refresh();

        void assign(const FilePath &p, std::error_code &ec);

        void replace_filename(const FilePath &p, std::error_code &ec);

        void refresh(std::error_code &ec) noexcept;

        // [fs.dir.entry.obs] observers
        const FilePath &path() const noexcept;

        operator const FilePath &() const noexcept;

        Result<bool> exists() const;

        Result<bool> is_block_file() const;

        Result<bool> is_character_file() const;

        Result<bool> is_directory() const;

        Result<bool> is_fifo() const;

        Result<bool> is_other() const;

        Result<bool> is_regular_file() const;

        Result<bool> is_socket() const;

        Result<bool> is_symlink() const;

        Result<uintmax_t> file_size() const;

        Result<Time> last_write_time() const;

        Result<FileStatus> status() const;

        Result<FileStatus> symlink_status() const;

        bool exists(std::error_code &ec) const noexcept;

        bool is_block_file(std::error_code &ec) const noexcept;

        bool is_character_file(std::error_code &ec) const noexcept;

        bool is_directory(std::error_code &ec) const noexcept;

        bool is_fifo(std::error_code &ec) const noexcept;

        bool is_other(std::error_code &ec) const noexcept;

        bool is_regular_file(std::error_code &ec) const noexcept;

        bool is_socket(std::error_code &ec) const noexcept;

        bool is_symlink(std::error_code &ec) const noexcept;

        uintmax_t file_size(std::error_code &ec) const noexcept;

        Time last_write_time(std::error_code &ec) const noexcept;

        FileStatus status(std::error_code &ec) const noexcept;

        FileStatus symlink_status(std::error_code &ec) const noexcept;

#ifndef OS_WEB

        Result<uintmax_t> hard_link_count() const;

        uintmax_t hard_link_count(std::error_code &ec) const noexcept;

#endif

        bool operator<(const DirectoryEntry &rhs) const noexcept;

        bool operator==(const DirectoryEntry &rhs) const noexcept;

        bool operator!=(const DirectoryEntry &rhs) const noexcept;

        bool operator<=(const DirectoryEntry &rhs) const noexcept;

        bool operator>(const DirectoryEntry &rhs) const noexcept;

        bool operator>=(const DirectoryEntry &rhs) const noexcept;

    private:
        friend class DirectoryIterator;


        Result<FileType> status_file_type() const;

        FileType status_file_type(std::error_code &ec) const noexcept;

        FilePath _path;
        FileStatus _status;
        FileStatus _symlink_status;
        uintmax_t _file_size = static_cast<uintmax_t>(-1);
#ifndef OS_WIN
        uintmax_t _hard_link_count = static_cast<uintmax_t>(-1);
#endif
        time_t _last_write_time = 0;
    };

    // [fs.class.directory.iterator] Class DirectoryIterator
    class TURBO_EXPORT DirectoryIterator {
    public:
        class TURBO_EXPORT proxy {
        public:
            const DirectoryEntry &operator*() const & noexcept { return _dir_entry; }

            DirectoryEntry operator*() && noexcept { return std::move(_dir_entry); }

        private:
            explicit proxy(const DirectoryEntry &dir_entry)
                    : _dir_entry(dir_entry) {
            }

            friend class DirectoryIterator;

            friend class RecursiveDirectoryIterator;

            DirectoryEntry _dir_entry;
        };

        using iterator_category = std::input_iterator_tag;
        using value_type = DirectoryEntry;
        using difference_type = std::ptrdiff_t;
        using pointer = const DirectoryEntry *;
        using reference = const DirectoryEntry &;

        // [fs.dir.itr.members] member functions
        DirectoryIterator() noexcept;


        static Result<DirectoryIterator> create(const FilePath &p, DirectoryOptions options = DirectoryOptions::none);


        DirectoryIterator(const FilePath &p, std::error_code &ec) noexcept;

        DirectoryIterator(const FilePath &p, DirectoryOptions options, std::error_code &ec) noexcept;

        DirectoryIterator(const DirectoryIterator &rhs);

        DirectoryIterator(DirectoryIterator &&rhs) noexcept;

        ~DirectoryIterator();

        DirectoryIterator &operator=(const DirectoryIterator &rhs);

        DirectoryIterator &operator=(DirectoryIterator &&rhs) noexcept;

        const DirectoryEntry &operator*() const;

        const DirectoryEntry *operator->() const;

#ifdef TURBO_FS_WITH_EXCEPTIONS

        DirectoryIterator &operator++();

#endif

        DirectoryIterator &increment(std::error_code &ec) noexcept;

        // other members as required by [input.iterators]
#ifdef TURBO_FS_WITH_EXCEPTIONS

        proxy operator++(int) {
            proxy p{**this};
            ++*this;
            return p;
        }

#endif

        bool operator==(const DirectoryIterator &rhs) const;

        bool operator!=(const DirectoryIterator &rhs) const;

    private:
        friend class RecursiveDirectoryIterator;

        class impl;

        std::shared_ptr<impl> _impl;
    };

    // [fs.dir.itr.nonmembers] DirectoryIterator non-member functions
    TURBO_EXPORT DirectoryIterator begin(DirectoryIterator iter) noexcept;

    TURBO_EXPORT DirectoryIterator end(const DirectoryIterator &) noexcept;

    // [fs.class.re.dir.itr] class RecursiveDirectoryIterator
    class TURBO_EXPORT RecursiveDirectoryIterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = DirectoryEntry;
        using difference_type = std::ptrdiff_t;
        using pointer = const DirectoryEntry *;
        using reference = const DirectoryEntry &;

        // [fs.rec.dir.itr.members] constructors and destructor
        RecursiveDirectoryIterator() noexcept;

        static Result<RecursiveDirectoryIterator>
        create(const FilePath &p, DirectoryOptions options = DirectoryOptions::none);


        RecursiveDirectoryIterator(const FilePath &p, DirectoryOptions options, std::error_code &ec) noexcept;

        RecursiveDirectoryIterator(const FilePath &p, std::error_code &ec) noexcept;

        RecursiveDirectoryIterator(const RecursiveDirectoryIterator &rhs);

        RecursiveDirectoryIterator(RecursiveDirectoryIterator &&rhs) noexcept;

        ~RecursiveDirectoryIterator();

        // [fs.rec.dir.itr.members] observers
        DirectoryOptions options() const;

        int depth() const;

        bool recursion_pending() const;

        const DirectoryEntry &operator*() const;

        const DirectoryEntry *operator->() const;

        // [fs.rec.dir.itr.members] modifiers RecursiveDirectoryIterator&
        RecursiveDirectoryIterator &operator=(const RecursiveDirectoryIterator &rhs);

        RecursiveDirectoryIterator &operator=(RecursiveDirectoryIterator &&rhs) noexcept;

#ifdef TURBO_FS_WITH_EXCEPTIONS

        RecursiveDirectoryIterator &operator++();

#endif

        RecursiveDirectoryIterator &increment(std::error_code &ec) noexcept;

        turbo::Status pop();

        void pop(std::error_code &ec);

        void disable_recursion_pending();

        // other members as required by [input.iterators]
#ifdef TURBO_FS_WITH_EXCEPTIONS

        DirectoryIterator::proxy operator++(int) {
            DirectoryIterator::proxy proxy{**this};
            ++*this;
            return proxy;
        }

#endif

        bool operator==(const RecursiveDirectoryIterator &rhs) const;

        bool operator!=(const RecursiveDirectoryIterator &rhs) const;

    private:
        struct RecursiveDirectoryIteratorImpl {
            DirectoryOptions _options;
            bool _recursion_pending;
            std::stack<DirectoryIterator> _dir_iter_stack;

            RecursiveDirectoryIteratorImpl(DirectoryOptions options, bool recursion_pending)
                    : _options(options), _recursion_pending(recursion_pending) {
            }
        };

        std::shared_ptr<RecursiveDirectoryIteratorImpl> _impl;
    };

    /// \brief File selector for filesystem APIs
    struct TURBO_EXPORT DirSelector {
        /// The directory in which to select files.
        /// If the path exists but doesn't point to a directory, this should be an error.
        FilePath base_dir;
        /// The behavior if `base_dir` isn't found in the filesystem.  If false,
        /// an error is returned.  If true, an empty selection is returned.
        bool allow_not_found{false};
        /// Whether to recurse into subdirectories.
        bool recursive{false};
        /// The maximum number of subdirectories to recurse into.
        int32_t max_recursion{INT32_MAX};

        /// collect result include sub file
        bool include_file{true};

        /// collect result include sub dir
        bool include_dir{true};

        /// sort result
        bool sort{false};

        DirectoryOptions dir_options{DirectoryOptions::none};

        /// return true if selected
        std::function<bool(const DirectoryEntry &entry)> filter;
    };


    // [fs.rec.dir.itr.nonmembers] DirectoryIterator non-member functions
    TURBO_EXPORT RecursiveDirectoryIterator begin(RecursiveDirectoryIterator iter) noexcept;

    TURBO_EXPORT RecursiveDirectoryIterator end(const RecursiveDirectoryIterator &) noexcept;

    // [fs.op.funcs] filesystem operations

    TURBO_EXPORT turbo::Result<FilePath> absolute(const FilePath &p);

    TURBO_EXPORT turbo::Result<FilePath> canonical(const FilePath &p);

    TURBO_EXPORT turbo::Status copy(const FilePath &from, const FilePath &to);

    TURBO_EXPORT Status copy(const FilePath &from, const FilePath &to, FileCopyOptions options);

    TURBO_EXPORT turbo::Result<bool> copy_file(const FilePath &from, const FilePath &to);

    TURBO_EXPORT turbo::Result<bool> copy_file(const FilePath &from, const FilePath &to, FileCopyOptions option);

    TURBO_EXPORT Status copy_symlink(const FilePath &existing_symlink, const FilePath &new_symlink);

    TURBO_EXPORT turbo::Result<bool> create_directories(const FilePath &p);

    TURBO_EXPORT turbo::Result<bool> create_directory(const FilePath &p);

    TURBO_EXPORT turbo::Result<bool> create_directory(const FilePath &p, const FilePath &attributes);

    TURBO_EXPORT Status create_directory_symlink(const FilePath &to, const FilePath &new_symlink);

    TURBO_EXPORT Status create_symlink(const FilePath &to, const FilePath &new_symlink);

    TURBO_EXPORT turbo::Result<FilePath> current_path();

    TURBO_EXPORT Status current_path(const FilePath &p);

    TURBO_EXPORT turbo::Result<bool> exists(const FilePath &p);

    TURBO_EXPORT turbo::Result<bool> equivalent(const FilePath &p1, const FilePath &p2);

    TURBO_EXPORT turbo::Result<uintmax_t> file_size(const FilePath &p);

    TURBO_EXPORT turbo::Result<bool> is_block_file(const FilePath &p);

    TURBO_EXPORT turbo::Result<bool> is_character_file(const FilePath &p);

    TURBO_EXPORT turbo::Result<bool> is_directory(const FilePath &p);

    TURBO_EXPORT turbo::Result<bool> is_empty(const FilePath &p);

    TURBO_EXPORT turbo::Result<bool> is_fifo(const FilePath &p);

    TURBO_EXPORT turbo::Result<bool> is_other(const FilePath &p);

    TURBO_EXPORT turbo::Result<bool> is_regular_file(const FilePath &p);

    TURBO_EXPORT turbo::Result<bool> is_socket(const FilePath &p);

    TURBO_EXPORT turbo::Result<bool> is_symlink(const FilePath &p);

    TURBO_EXPORT Result<Time> last_write_time(const FilePath &p);

    TURBO_EXPORT Status last_write_time(const FilePath &p, Time new_time);

    TURBO_EXPORT Status permissions(const FilePath &p, FilePerms prms, FilePermOptions opts = FilePermOptions::replace);

    ///recursive
    TURBO_EXPORT Result<std::vector<FilePath>> list_files(const DirSelector &p);

    TURBO_EXPORT Result<std::vector<FilePath>> list_files(const FilePath &p, bool recursive = false);

    TURBO_EXPORT Result<std::vector<DirectoryEntry>> list_entry(const DirSelector &p);

    TURBO_EXPORT Result<std::vector<DirectoryEntry>> list_entry(const FilePath &p, bool recursive = false);

    ///  const FilePath &base = current_path()
    TURBO_EXPORT turbo::Result<FilePath> proximate(const FilePath &p);

    TURBO_EXPORT turbo::Result<FilePath> proximate(const FilePath &p, const FilePath &base);

    TURBO_EXPORT turbo::Result<FilePath> read_symlink(const FilePath &p);
    /// , const FilePath &base = current_path()
    TURBO_EXPORT turbo::Result<FilePath> relative(const FilePath &p);

    TURBO_EXPORT turbo::Result<FilePath> relative(const FilePath &p, const FilePath &base);

    TURBO_EXPORT turbo::Result<bool> remove(const FilePath &p);

    TURBO_EXPORT turbo::Result<uintmax_t> remove_all(const FilePath &p);

    TURBO_EXPORT Status rename(const FilePath &from, const FilePath &to);

    TURBO_EXPORT Status resize_file(const FilePath &p, uintmax_t size);

    TURBO_EXPORT turbo::Result<FileSpaceInfo> space(const FilePath &p);

    TURBO_EXPORT turbo::Result<FileStatus> status(const FilePath &p);

    TURBO_EXPORT turbo::Result<FileStatus> symlink_status(const FilePath &p);

    TURBO_EXPORT turbo::Result<FilePath> temp_directory_path();

    TURBO_EXPORT turbo::Result<FilePath> weakly_canonical(const FilePath &p);


    /// with std::error_code
    TURBO_EXPORT FilePath absolute(const FilePath &p, std::error_code &ec);

    TURBO_EXPORT FilePath canonical(const FilePath &p, std::error_code &ec);

    TURBO_EXPORT void copy(const FilePath &from, const FilePath &to, std::error_code &ec) noexcept;

    TURBO_EXPORT void
    copy(const FilePath &from, const FilePath &to, FileCopyOptions options, std::error_code &ec) noexcept;

    TURBO_EXPORT bool copy_file(const FilePath &from, const FilePath &to, std::error_code &ec) noexcept;

    TURBO_EXPORT bool
    copy_file(const FilePath &from, const FilePath &to, FileCopyOptions option, std::error_code &ec) noexcept;

    TURBO_EXPORT void
    copy_symlink(const FilePath &existing_symlink, const FilePath &new_symlink, std::error_code &ec) noexcept;

    TURBO_EXPORT bool create_directories(const FilePath &p, std::error_code &ec) noexcept;

    TURBO_EXPORT bool create_directory(const FilePath &p, std::error_code &ec) noexcept;

    TURBO_EXPORT bool create_directory(const FilePath &p, const FilePath &attributes, std::error_code &ec) noexcept;

    TURBO_EXPORT void
    create_directory_symlink(const FilePath &to, const FilePath &new_symlink, std::error_code &ec) noexcept;

    TURBO_EXPORT void create_symlink(const FilePath &to, const FilePath &new_symlink, std::error_code &ec) noexcept;

    TURBO_EXPORT FilePath current_path(std::error_code &ec);

    TURBO_EXPORT void current_path(const FilePath &p, std::error_code &ec) noexcept;

    TURBO_EXPORT bool exists(FileStatus s) noexcept;

    TURBO_EXPORT bool exists(const FilePath &p, std::error_code &ec) noexcept;

    TURBO_EXPORT bool equivalent(const FilePath &p1, const FilePath &p2, std::error_code &ec) noexcept;

    TURBO_EXPORT uintmax_t file_size(const FilePath &p, std::error_code &ec) noexcept;

    TURBO_EXPORT bool is_block_file(FileStatus s) noexcept;

    TURBO_EXPORT bool is_block_file(const FilePath &p, std::error_code &ec) noexcept;

    TURBO_EXPORT bool is_character_file(FileStatus s) noexcept;

    TURBO_EXPORT bool is_character_file(const FilePath &p, std::error_code &ec) noexcept;

    TURBO_EXPORT bool is_directory(FileStatus s) noexcept;

    TURBO_EXPORT bool is_directory(const FilePath &p, std::error_code &ec) noexcept;

    TURBO_EXPORT bool is_empty(const FilePath &p, std::error_code &ec) noexcept;

    TURBO_EXPORT bool is_fifo(FileStatus s) noexcept;

    TURBO_EXPORT bool is_fifo(const FilePath &p, std::error_code &ec) noexcept;

    TURBO_EXPORT bool is_other(FileStatus s) noexcept;

    TURBO_EXPORT bool is_other(const FilePath &p, std::error_code &ec) noexcept;

    TURBO_EXPORT bool is_regular_file(FileStatus s) noexcept;

    TURBO_EXPORT bool is_regular_file(const FilePath &p, std::error_code &ec) noexcept;

    TURBO_EXPORT bool is_socket(FileStatus s) noexcept;

    TURBO_EXPORT bool is_socket(const FilePath &p, std::error_code &ec) noexcept;

    TURBO_EXPORT bool is_symlink(FileStatus s) noexcept;

    TURBO_EXPORT bool is_symlink(const FilePath &p, std::error_code &ec) noexcept;

    TURBO_EXPORT Time last_write_time(const FilePath &p, std::error_code &ec) noexcept;

    TURBO_EXPORT void last_write_time(const FilePath &p, Time new_time, std::error_code &ec) noexcept;

    TURBO_EXPORT void permissions(const FilePath &p, FilePerms prms, std::error_code &ec) noexcept;

    TURBO_EXPORT void
    permissions(const FilePath &p, FilePerms prms, FilePermOptions opts, std::error_code &ec) noexcept;

    TURBO_EXPORT FilePath proximate(const FilePath &p, std::error_code &ec);

    TURBO_EXPORT FilePath proximate(const FilePath &p, const FilePath &base, std::error_code &ec);

    TURBO_EXPORT FilePath read_symlink(const FilePath &p, std::error_code &ec);

    TURBO_EXPORT FilePath relative(const FilePath &p, std::error_code &ec);

    TURBO_EXPORT FilePath relative(const FilePath &p, const FilePath &base, std::error_code &ec);

    TURBO_EXPORT bool remove(const FilePath &p, std::error_code &ec) noexcept;

    TURBO_EXPORT uintmax_t remove_all(const FilePath &p, std::error_code &ec) noexcept;

    TURBO_EXPORT void rename(const FilePath &from, const FilePath &to, std::error_code &ec) noexcept;

    TURBO_EXPORT void resize_file(const FilePath &p, uintmax_t size, std::error_code &ec) noexcept;

    TURBO_EXPORT FileSpaceInfo space(const FilePath &p, std::error_code &ec) noexcept;

    TURBO_EXPORT FileStatus status(const FilePath &p, std::error_code &ec) noexcept;

    TURBO_EXPORT bool status_known(FileStatus s) noexcept;

    TURBO_EXPORT FileStatus symlink_status(const FilePath &p, std::error_code &ec) noexcept;

    TURBO_EXPORT FilePath temp_directory_path(std::error_code &ec) noexcept;

    TURBO_EXPORT FilePath weakly_canonical(const FilePath &p, std::error_code &ec) noexcept;

#ifndef OS_WEB

    TURBO_EXPORT Status create_hard_link(const FilePath &to, const FilePath &new_hard_link);

    TURBO_EXPORT Result<uintmax_t> hard_link_count(const FilePath &p);


    TURBO_EXPORT void create_hard_link(const FilePath &to, const FilePath &new_hard_link, std::error_code &ec) noexcept;

    TURBO_EXPORT uintmax_t hard_link_count(const FilePath &p, std::error_code &ec) noexcept;

#endif

#if defined(OS_WIN) && (!defined(__GLIBCXX__) || (defined(_GLIBCXX_HAVE__WFOPEN) && defined(_GLIBCXX_USE_WCHAR_T)))
#define GHC_HAS_FSTREAM_OPEN_WITH_WCHAR
#endif

    // Non-C++17 add-on std::fstream wrappers with path
    template<class charT, class traits = std::char_traits<charT>>
    class basic_filebuf : public std::basic_filebuf<charT, traits> {
    public:
        basic_filebuf() {}

        ~basic_filebuf() override {}

        basic_filebuf(const basic_filebuf &) = delete;

        const basic_filebuf &operator=(const basic_filebuf &) = delete;

        basic_filebuf<charT, traits> *open(const FilePath &p, std::ios_base::openmode mode) {
#ifdef GHC_HAS_FSTREAM_OPEN_WITH_WCHAR
            return std::basic_filebuf<charT, traits>::open(p.wstring().c_str(), mode) ? this : 0;
#else
            return std::basic_filebuf<charT, traits>::open(p.string().c_str(), mode) ? this : 0;
#endif
        }
    };

    template<class charT, class traits = std::char_traits<charT>>
    class basic_ifstream : public std::basic_ifstream<charT, traits> {
    public:
        basic_ifstream() {}

#ifdef GHC_HAS_FSTREAM_OPEN_WITH_WCHAR
        explicit basic_ifstream(const path& p, std::ios_base::openmode mode = std::ios_base::in)
            : std::basic_ifstream<charT, traits>(p.wstring().c_str(), mode)
        {
        }
        void open(const path& p, std::ios_base::openmode mode = std::ios_base::in) { std::basic_ifstream<charT, traits>::open(p.wstring().c_str(), mode); }
#else

        explicit basic_ifstream(const FilePath &p, std::ios_base::openmode mode = std::ios_base::in)
                : std::basic_ifstream<charT, traits>(p.string().c_str(), mode) {
        }

        void open(const FilePath &p, std::ios_base::openmode mode = std::ios_base::in) {
            std::basic_ifstream<charT, traits>::open(p.string().c_str(), mode);
        }

#endif

        basic_ifstream(const basic_ifstream &) = delete;

        const basic_ifstream &operator=(const basic_ifstream &) = delete;

        ~basic_ifstream() override {}
    };

    template<class charT, class traits = std::char_traits<charT>>
    class basic_ofstream : public std::basic_ofstream<charT, traits> {
    public:
        basic_ofstream() {}

#ifdef GHC_HAS_FSTREAM_OPEN_WITH_WCHAR
        explicit basic_ofstream(const path& p, std::ios_base::openmode mode = std::ios_base::out)
            : std::basic_ofstream<charT, traits>(p.wstring().c_str(), mode)
        {
        }
        void open(const path& p, std::ios_base::openmode mode = std::ios_base::out) { std::basic_ofstream<charT, traits>::open(p.wstring().c_str(), mode); }
#else

        explicit basic_ofstream(const FilePath &p, std::ios_base::openmode mode = std::ios_base::out)
                : std::basic_ofstream<charT, traits>(p.string().c_str(), mode) {
        }

        void open(const FilePath &p, std::ios_base::openmode mode = std::ios_base::out) {
            std::basic_ofstream<charT, traits>::open(p.string().c_str(), mode);
        }

#endif

        basic_ofstream(const basic_ofstream &) = delete;

        const basic_ofstream &operator=(const basic_ofstream &) = delete;

        ~basic_ofstream() override {}
    };

    template<class charT, class traits = std::char_traits<charT>>
    class basic_fstream : public std::basic_fstream<charT, traits> {
    public:
        basic_fstream() {}

#ifdef GHC_HAS_FSTREAM_OPEN_WITH_WCHAR
        explicit basic_fstream(const path& p, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out)
            : std::basic_fstream<charT, traits>(p.wstring().c_str(), mode)
        {
        }
        void open(const path& p, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) { std::basic_fstream<charT, traits>::open(p.wstring().c_str(), mode); }
#else

        explicit basic_fstream(const FilePath &p, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out)
                : std::basic_fstream<charT, traits>(p.string().c_str(), mode) {
        }

        void open(const FilePath &p, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) {
            std::basic_fstream<charT, traits>::open(p.string().c_str(), mode);
        }

#endif

        basic_fstream(const basic_fstream &) = delete;

        const basic_fstream &operator=(const basic_fstream &) = delete;

        ~basic_fstream() override {}
    };

    typedef basic_filebuf<char> filebuf;
    typedef basic_filebuf<wchar_t> wfilebuf;
    typedef basic_ifstream<char> ifstream;
    typedef basic_ifstream<wchar_t> wifstream;
    typedef basic_ofstream<char> ofstream;
    typedef basic_ofstream<wchar_t> wofstream;
    typedef basic_fstream<char> fstream;
    typedef basic_fstream<wchar_t> wfstream;

    class TURBO_EXPORT u8arguments {
    public:
        u8arguments(int &argc, char **&argv);

        ~u8arguments() {
            _refargc = _argc;
            _refargv = _argv;
        }

        bool valid() const { return _isvalid; }

    private:
        int _argc;
        char **_argv;
        int &_refargc;
        char **&_refargv;
        bool _isvalid;
#ifdef OS_WIN
        std::vector<std::string> _args;
        std::vector<char*> _argp;
#endif
    };

//-------------------------------------------------------------------------------------------------
//  Implementation
//-------------------------------------------------------------------------------------------------

    namespace detail {
        enum utf8_states_t {
            S_STRT = 0, S_RJCT = 8
        };

        TURBO_EXPORT void appendUTF8(std::string &str, uint32_t unicode);

        TURBO_EXPORT bool is_surrogate(uint32_t c);

        TURBO_EXPORT bool is_high_surrogate(uint32_t c);

        TURBO_EXPORT bool is_low_surrogate(uint32_t c);

        TURBO_EXPORT unsigned
        consumeUtf8Fragment(const unsigned state, const uint8_t fragment, uint32_t &codepoint);

        enum class portable_error {
            none = 0,
            exists,
            not_found,
            not_supported,
            not_implemented,
            invalid_argument,
            is_a_directory,
        };

        TURBO_EXPORT std::error_code make_error_code(portable_error err);

#ifdef OS_WIN
        TURBO_EXPORT std::error_code make_system_error(uint32_t err = 0);
#else

        TURBO_EXPORT std::error_code make_system_error(int err = 0);

        template<typename T, typename = int>
        struct has_d_type : std::false_type {
        };

        template<typename T>
        struct has_d_type<T, decltype((void) T::d_type, 0)> : std::true_type {
        };

        template<typename T>
        inline FileType file_type_from_dirent_impl(const T &, std::false_type) {
            return FileType::none;
        }

        template<typename T>
        inline FileType file_type_from_dirent_impl(const T &t, std::true_type) {
            switch (t.d_type) {
#ifdef DT_BLK
                case DT_BLK:
                    return FileType::block;
#endif
#ifdef DT_CHR
                case DT_CHR:
                    return FileType::character;
#endif
#ifdef DT_DIR
                case DT_DIR:
                    return FileType::directory;
#endif
#ifdef DT_FIFO
                case DT_FIFO:
                    return FileType::fifo;
#endif
#ifdef DT_LNK
                case DT_LNK:
                    return FileType::symlink;
#endif
#ifdef DT_REG
                case DT_REG:
                    return FileType::regular;
#endif
#ifdef DT_SOCK
                case DT_SOCK:
                    return FileType::socket;
#endif
#ifdef DT_UNKNOWN
                case DT_UNKNOWN:
                    return FileType::none;
#endif
                default:
                    return FileType::unknown;
            }
        }

        template<class T>
        inline FileType file_type_from_dirent(const T &t) {
            return file_type_from_dirent_impl(t, has_d_type < T > {});
        }

#endif
    }  // namespace detail

    namespace detail {

        template<typename Enum>
        using EnableBitmask = typename std::enable_if<
                std::is_same<Enum, FilePerms>::value || std::is_same<Enum, FilePermOptions>::value ||
                std::is_same<Enum, FileCopyOptions>::value ||
                std::is_same<Enum, DirectoryOptions>::value, Enum>::type;
    }  // namespace detail

    template<typename Enum>
    constexpr detail::EnableBitmask<Enum> operator&(Enum X, Enum Y) {
        using underlying = typename std::underlying_type<Enum>::type;
        return static_cast<Enum>(static_cast<underlying>(X) & static_cast<underlying>(Y));
    }

    template<typename Enum>
    constexpr detail::EnableBitmask<Enum> operator|(Enum X, Enum Y) {
        using underlying = typename std::underlying_type<Enum>::type;
        return static_cast<Enum>(static_cast<underlying>(X) | static_cast<underlying>(Y));
    }

    template<typename Enum>
    constexpr detail::EnableBitmask<Enum> operator^(Enum X, Enum Y) {
        using underlying = typename std::underlying_type<Enum>::type;
        return static_cast<Enum>(static_cast<underlying>(X) ^ static_cast<underlying>(Y));
    }

    template<typename Enum>
    constexpr detail::EnableBitmask<Enum> operator~(Enum X) {
        using underlying = typename std::underlying_type<Enum>::type;
        return static_cast<Enum>(~static_cast<underlying>(X));
    }

    template<typename Enum>
    detail::EnableBitmask<Enum> &operator&=(Enum &X, Enum Y) {
        X = X & Y;
        return X;
    }

    template<typename Enum>
    detail::EnableBitmask<Enum> &operator|=(Enum &X, Enum Y) {
        X = X | Y;
        return X;
    }

    template<typename Enum>
    detail::EnableBitmask<Enum> &operator^=(Enum &X, Enum Y) {
        X = X ^ Y;
        return X;
    }


    namespace detail {

        inline bool in_range(uint32_t c, uint32_t lo, uint32_t hi) {
            return (static_cast<uint32_t>(c - lo) < (hi - lo + 1));
        }

        inline bool is_surrogate(uint32_t c) {
            return in_range(c, 0xd800, 0xdfff);
        }

        inline bool is_high_surrogate(uint32_t c) {
            return (c & 0xfffffc00) == 0xd800;
        }

        inline bool is_low_surrogate(uint32_t c) {
            return (c & 0xfffffc00) == 0xdc00;
        }

        inline void appendUTF8(std::string &str, uint32_t unicode) {
            if (unicode <= 0x7f) {
                str.push_back(static_cast<char>(unicode));
            } else if (unicode >= 0x80 && unicode <= 0x7ff) {
                str.push_back(static_cast<char>((unicode >> 6) + 192));
                str.push_back(static_cast<char>((unicode & 0x3f) + 128));
            } else if ((unicode >= 0x800 && unicode <= 0xd7ff) || (unicode >= 0xe000 && unicode <= 0xffff)) {
                str.push_back(static_cast<char>((unicode >> 12) + 224));
                str.push_back(static_cast<char>(((unicode & 0xfff) >> 6) + 128));
                str.push_back(static_cast<char>((unicode & 0x3f) + 128));
            } else if (unicode >= 0x10000 && unicode <= 0x10ffff) {
                str.push_back(static_cast<char>((unicode >> 18) + 240));
                str.push_back(static_cast<char>(((unicode & 0x3ffff) >> 12) + 128));
                str.push_back(static_cast<char>(((unicode & 0xfff) >> 6) + 128));
                str.push_back(static_cast<char>((unicode & 0x3f) + 128));
            } else {
                appendUTF8(str, 0xfffd);
            }
        }

// Thanks to Bjoern Hoehrmann (https://bjoern.hoehrmann.de/utf-8/decoder/dfa/)
// and Taylor R Campbell for the ideas to this DFA approach of UTF-8 decoding;
// Generating debugging and shrinking my own DFA from scratch was a day of fun!
        inline unsigned consumeUtf8Fragment(const unsigned state, const uint8_t fragment, uint32_t &codepoint) {
            static const uint32_t utf8_state_info[] = {
                    // encoded states
                    0x11111111u, 0x11111111u, 0x77777777u, 0x77777777u, 0x88888888u, 0x88888888u, 0x88888888u,
                    0x88888888u, 0x22222299u, 0x22222222u, 0x22222222u, 0x22222222u, 0x3333333au, 0x33433333u,
                    0x9995666bu, 0x99999999u,
                    0x88888880u, 0x22818108u, 0x88888881u, 0x88888882u, 0x88888884u, 0x88888887u, 0x88888886u,
                    0x82218108u, 0x82281108u, 0x88888888u, 0x88888883u, 0x88888885u, 0u, 0u, 0u, 0u,
            };
            uint8_t category =
                    fragment < 128 ? 0 : (utf8_state_info[(fragment >> 3) & 0xf] >> ((fragment & 7) << 2)) & 0xf;
            codepoint = (state ? (codepoint << 6) | (fragment & 0x3fu) : (0xffu >> category) & fragment);
            return state == S_RJCT ? static_cast<unsigned>(S_RJCT) : static_cast<unsigned>(
                    (utf8_state_info[category + 16] >> (state << 2)) & 0xf);
        }

        inline bool validUtf8(const std::string &utf8String) {
            std::string::const_iterator iter = utf8String.begin();
            unsigned utf8_state = S_STRT;
            std::uint32_t codepoint = 0;
            while (iter < utf8String.end()) {
                if ((utf8_state = consumeUtf8Fragment(utf8_state, static_cast<uint8_t>(*iter++), codepoint)) ==
                    S_RJCT) {
                    return false;
                }
            }
            if (utf8_state) {
                return false;
            }
            return true;
        }

    }  // namespace detail


    namespace detail {

        template<class StringType, class Utf8String, typename std::enable_if<
                FilePath::_is_basic_string<Utf8String>::value && (sizeof(typename Utf8String::value_type) == 1) &&
                (sizeof(typename StringType::value_type) == 1)>::type * = nullptr>
        inline StringType fromUtf8(const Utf8String &utf8String,
                                   const typename StringType::allocator_type &alloc = typename StringType::allocator_type()) {
            return StringType(utf8String.begin(), utf8String.end(), alloc);
        }

        template<class StringType, class Utf8String, typename std::enable_if<
                FilePath::_is_basic_string<Utf8String>::value && (sizeof(typename Utf8String::value_type) == 1) &&
                (sizeof(typename StringType::value_type) == 2)>::type * = nullptr>
        inline StringType fromUtf8(const Utf8String &utf8String,
                                   const typename StringType::allocator_type &alloc = typename StringType::allocator_type()) {
            StringType result(alloc);
            result.reserve(utf8String.length());
            auto iter = utf8String.cbegin();
            unsigned utf8_state = S_STRT;
            std::uint32_t codepoint = 0;
            while (iter < utf8String.cend()) {
                if ((utf8_state = consumeUtf8Fragment(utf8_state, static_cast<uint8_t>(*iter++), codepoint)) ==
                    S_STRT) {
                    if (codepoint <= 0xffff) {
                        result += static_cast<typename StringType::value_type>(codepoint);
                    } else {
                        codepoint -= 0x10000;
                        result += static_cast<typename StringType::value_type>((codepoint >> 10) + 0xd800);
                        result += static_cast<typename StringType::value_type>((codepoint & 0x3ff) + 0xdc00);
                    }
                    codepoint = 0;
                } else if (utf8_state == S_RJCT) {
                    result += static_cast<typename StringType::value_type>(0xfffd);
                    utf8_state = S_STRT;
                    codepoint = 0;
                }
            }
            if (utf8_state) {
                result += static_cast<typename StringType::value_type>(0xfffd);
            }
            return result;
        }

        template<class StringType, class Utf8String, typename std::enable_if<
                FilePath::_is_basic_string<Utf8String>::value && (sizeof(typename Utf8String::value_type) == 1) &&
                (sizeof(typename StringType::value_type) == 4)>::type * = nullptr>
        inline StringType fromUtf8(const Utf8String &utf8String,
                                   const typename StringType::allocator_type &alloc = typename StringType::allocator_type()) {
            StringType result(alloc);
            result.reserve(utf8String.length());
            auto iter = utf8String.cbegin();
            unsigned utf8_state = S_STRT;
            std::uint32_t codepoint = 0;
            while (iter < utf8String.cend()) {
                if ((utf8_state = consumeUtf8Fragment(utf8_state, static_cast<uint8_t>(*iter++), codepoint)) ==
                    S_STRT) {
                    result += static_cast<typename StringType::value_type>(codepoint);
                    codepoint = 0;
                } else if (utf8_state == S_RJCT) {
                    result += static_cast<typename StringType::value_type>(0xfffd);
                    utf8_state = S_STRT;
                    codepoint = 0;
                }
            }
            if (utf8_state) {
                result += static_cast<typename StringType::value_type>(0xfffd);
            }
            return result;
        }

        template<class StringType, typename charT, std::size_t N>
        inline StringType fromUtf8(const charT (&utf8String)[N]) {
            return fromUtf8 < StringType > (basic_string_view<charT>(utf8String, N - 1));
        }

        template<typename strT, typename std::enable_if<FilePath::_is_basic_string<strT>::value &&
                                                        (sizeof(typename strT::value_type) ==
                                                         1), int>::type size = 1>
        inline std::string toUtf8(const strT &unicodeString) {
            return std::string(unicodeString.begin(), unicodeString.end());
        }

        template<typename strT, typename std::enable_if<FilePath::_is_basic_string<strT>::value &&
                                                        (sizeof(typename strT::value_type) ==
                                                         2), int>::type size = 2>
        inline std::string toUtf8(const strT &unicodeString) {
            std::string result;
            for (auto iter = unicodeString.begin(); iter != unicodeString.end(); ++iter) {
                char32_t c = *iter;
                if (is_surrogate(c)) {
                    ++iter;
                    if (iter != unicodeString.end() && is_high_surrogate(c) && is_low_surrogate(*iter)) {
                        appendUTF8(result, (char32_t(c) << 10) + *iter - 0x35fdc00);
                    } else {
                        appendUTF8(result, 0xfffd);
                        if (iter == unicodeString.end()) {
                            break;
                        }
                    }
                } else {
                    appendUTF8(result, c);
                }
            }
            return result;
        }

        template<typename strT, typename std::enable_if<FilePath::_is_basic_string<strT>::value &&
                                                        (sizeof(typename strT::value_type) ==
                                                         4), int>::type size = 4>
        inline std::string toUtf8(const strT &unicodeString) {
            std::string result;
            for (auto c: unicodeString) {
                appendUTF8(result, static_cast<uint32_t>(c));
            }
            return result;
        }

        template<typename charT>
        inline std::string toUtf8(const charT *unicodeString) {
            return toUtf8(basic_string_view<charT, std::char_traits<charT>>(unicodeString));
        }

#ifdef GHC_USE_WCHAR_T
        template <class StringType, class WString, typename std::enable_if<path::_is_basic_string<WString>::value && (sizeof(typename WString::value_type) == 2) && (sizeof(typename StringType::value_type) == 1), bool>::type = false>
        inline StringType fromWChar(const WString& wString, const typename StringType::allocator_type& alloc = typename StringType::allocator_type())
        {
            auto temp = toUtf8(wString);
            return StringType(temp.begin(), temp.end(), alloc);
        }

        template <class StringType, class WString, typename std::enable_if<path::_is_basic_string<WString>::value && (sizeof(typename WString::value_type) == 2) && (sizeof(typename StringType::value_type) == 2), bool>::type = false>
        inline StringType fromWChar(const WString& wString, const typename StringType::allocator_type& alloc = typename StringType::allocator_type())
        {
            return StringType(wString.begin(), wString.end(), alloc);
        }

        template <class StringType, class WString, typename std::enable_if<path::_is_basic_string<WString>::value && (sizeof(typename WString::value_type) == 2) && (sizeof(typename StringType::value_type) == 4), bool>::type = false>
        inline StringType fromWChar(const WString& wString, const typename StringType::allocator_type& alloc = typename StringType::allocator_type())
        {
            auto temp = toUtf8(wString);
            return fromUtf8<StringType>(temp, alloc);
        }

        template <typename strT, typename std::enable_if<path::_is_basic_string<strT>::value && (sizeof(typename strT::value_type) == 1), bool>::type = false>
        inline std::wstring toWChar(const strT& unicodeString)
        {
            return fromUtf8<std::wstring>(unicodeString);
        }

        template <typename strT, typename std::enable_if<path::_is_basic_string<strT>::value && (sizeof(typename strT::value_type) == 2), bool>::type = false>
        inline std::wstring toWChar(const strT& unicodeString)
        {
            return std::wstring(unicodeString.begin(), unicodeString.end());
        }

        template <typename strT, typename std::enable_if<path::_is_basic_string<strT>::value && (sizeof(typename strT::value_type) == 4), bool>::type = false>
        inline std::wstring toWChar(const strT& unicodeString)
        {
            auto temp = toUtf8(unicodeString);
            return fromUtf8<std::wstring>(temp);
        }

        template <typename charT>
        inline std::wstring toWChar(const charT* unicodeString)
        {
            return toWChar(basic_string_view<charT, std::char_traits<charT>>(unicodeString));
        }
#endif  // GHC_USE_WCHAR_T

    }  // namespace detail

    template<class Source, typename>
    inline FilePath::FilePath(const Source &source, format fmt)
#ifdef GHC_USE_WCHAR_T
    : _path(detail::toWChar(source))
#else
            : _path(detail::toUtf8(source))
#endif
    {
        postprocess_path_with_format(fmt);
    }

    template<class Source, typename>
    inline FilePath u8path(const Source &source) {
        return path(source);
    }

    template<class InputIterator>
    inline FilePath u8path(InputIterator first, InputIterator last) {
        return path(first, last);
    }

    template<class InputIterator>
    inline FilePath::FilePath(InputIterator first, InputIterator last, format fmt)
            : FilePath(std::basic_string<typename std::iterator_traits<InputIterator>::value_type>(first, last), fmt) {
        // delegated
    }


    template<class Source, typename>
    Result<FilePath> FilePath::create(const Source &source, const std::locale &loc, format fmt) {
        FilePath p(source, fmt);
        std::string locName = loc.name();
        if (!(locName.length() >= 5 && (locName.substr(locName.length() - 5) == "UTF-8" ||
                                        locName.substr(locName.length() - 5) == "utf-8"))) {
            return invalid_argument_error("This implementation only supports UTF-8 locales!", p._path,
                                          detail::make_error_code(detail::portable_error::not_supported));
        }
        return p;

    }

    template<class InputIterator>
    Result<FilePath> FilePath::create(InputIterator first, InputIterator last, const std::locale &loc, format fmt) {
        FilePath p(std::basic_string<typename std::iterator_traits<InputIterator>::value_type>(first, last), fmt);
        std::string locName = loc.name();
        if (!(locName.length() >= 5 && (locName.substr(locName.length() - 5) == "UTF-8" ||
                                        locName.substr(locName.length() - 5) == "utf-8"))) {
            return invalid_argument_error("This implementation only supports UTF-8 locales!", p._path,
                                          detail::make_error_code(detail::portable_error::not_supported));
        }
        return p;
    }


    template<class Source>
    inline FilePath &FilePath::operator=(const Source &source) {
        return assign(source);
    }

    template<class Source>
    inline FilePath &FilePath::assign(const Source &source) {
#ifdef GHC_USE_WCHAR_T
        _path.assign(detail::toWChar(source));
#else
        _path.assign(detail::toUtf8(source));
#endif
        postprocess_path_with_format(native_format);
        return *this;
    }

    template<>
    inline FilePath &FilePath::assign<FilePath>(const FilePath &source) {
        _path = source._path;
#if defined(OS_WIN)
        _prefixLength = source._prefixLength;
#endif
        return *this;
    }

    template<class InputIterator>
    inline FilePath &FilePath::assign(InputIterator first, InputIterator last) {
        _path.assign(first, last);
        postprocess_path_with_format(native_format);
        return *this;
    }

    template<class Source>
    inline FilePath &FilePath::operator/=(const Source &source) {
        return append(source);
    }

    template<class Source>
    inline FilePath &FilePath::append(const Source &source) {
        return this->operator/=(FilePath(source));
    }

    template<>
    inline FilePath &FilePath::append<FilePath>(const FilePath &p) {
        return this->operator/=(p);
    }

    template<class InputIterator>
    inline FilePath &FilePath::append(InputIterator first, InputIterator last) {
        std::basic_string<typename std::iterator_traits<InputIterator>::value_type> part(first, last);
        return append(part);
    }


    template<class Source>
    inline FilePath::path_from_string<Source> &FilePath::operator+=(const Source &x) {
        return concat(x);
    }

    template<class EcharT>
    inline FilePath::path_type_EcharT<EcharT> &FilePath::operator+=(EcharT x) {
        basic_string_view<EcharT> part(&x, 1);
        concat(part);
        return *this;
    }

    template<class Source>
    inline FilePath &FilePath::concat(const Source &x) {
        FilePath p(x);
        _path += p._path;
        postprocess_path_with_format(native_format);
        return *this;
    }

    template<class InputIterator>
    inline FilePath &FilePath::concat(InputIterator first, InputIterator last) {
        _path.append(first, last);
        postprocess_path_with_format(native_format);
        return *this;
    }


    template<class EcharT, class traits, class Allocator>
    inline std::basic_string<EcharT, traits, Allocator> FilePath::string(const Allocator &a) const {
#ifdef GHC_USE_WCHAR_T
        return detail::fromWChar<std::basic_string<EcharT, traits, Allocator>>(_path, a);
#else
        return detail::fromUtf8<std::basic_string<EcharT, traits, Allocator>>(_path, a);
#endif
    }

    //-----------------------------------------------------------------------------
    // [fs.path.generic.obs] generic format observers
    template<class EcharT, class traits, class Allocator>
    inline std::basic_string<EcharT, traits, Allocator> FilePath::generic_string(const Allocator &a) const {
#ifdef OS_WIN
#ifdef GHC_USE_WCHAR_T
        auto result = detail::fromWChar<std::basic_string<EcharT, traits, Allocator>, path::string_type>(_path, a);
#else
        auto result = detail::fromUtf8<std::basic_string<EcharT, traits, Allocator>>(_path, a);
#endif
        for (auto& c : result) {
            if (c == preferred_separator) {
                c = generic_separator;
            }
        }
        return result;
#else
        return detail::fromUtf8<std::basic_string<EcharT, traits, Allocator>>(_path, a);
#endif
    }

    //-----------------------------------------------------------------------------
    // [fs.path.io] path inserter and extractor
    template<class charT, class traits>
    inline std::basic_ostream<charT, traits> &operator<<(std::basic_ostream<charT, traits> &os, const FilePath &p) {
        os << "\"";
        auto ps = p.string<charT, traits>();
        for (auto c: ps) {
            if (c == '"' || c == '\\') {
                os << '\\';
            }
            os << c;
        }
        os << "\"";
        return os;
    }

    template<class charT, class traits>
    inline std::basic_istream<charT, traits> &operator>>(std::basic_istream<charT, traits> &is, FilePath &p) {
        std::basic_string<charT, traits> tmp;
        charT c;
        is >> c;
        if (c == '"') {
            auto sf = is.flags();
            is >> std::noskipws;
            while (is) {
                auto c2 = is.get();
                if (is) {
                    if (c2 == '\\') {
                        c2 = is.get();
                        if (is) {
                            tmp += static_cast<charT>(c2);
                        }
                    } else if (c2 == '"') {
                        break;
                    } else {
                        tmp += static_cast<charT>(c2);
                    }
                }
            }
            if ((sf & std::ios_base::skipws) == std::ios_base::skipws) {
                is >> std::skipws;
            }
            p = path(tmp);
        } else {
            is >> tmp;
            p = path(static_cast<charT>(c) + tmp);
        }
        return is;
    }

}  // namespace turbo

