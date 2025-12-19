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

#include <turbo/files/dir_enumerator.h>

#include <dirent.h>
#include <cerrno>
#include <fnmatch.h>

#include <turbo/log/logging.h>
#include <turbo/threading/thread_restrictions.h>

namespace turbo {

    // DirEnumerator::FileInfo ----------------------------------------------------

    DirEnumerator::FileInfo::FileInfo() {
        memset(&stat_, 0, sizeof(stat_));
    }

    bool DirEnumerator::FileInfo::IsDirectory() const {
        return S_ISDIR(stat_.st_mode);
    }

    turbo::FilePath DirEnumerator::FileInfo::GetName() const {
        return filename_;
    }

    int64_t DirEnumerator::FileInfo::GetSize() const {
        return stat_.st_size;
    }

    turbo::Time DirEnumerator::FileInfo::GetLastModifiedTime() const {
        return turbo::Time::from_time_t(stat_.st_mtime);
    }

    // DirEnumerator --------------------------------------------------------------

    DirEnumerator::DirEnumerator(const turbo::FilePath &root_path,
                                   bool recursive,
                                   int FileType)
            : current_directory_entry_(0),
              root_path_(root_path),
              recursive_(recursive),
              file_type_(FileType) {
        // INCLUDE_DOT_DOT must not be specified if recursive.
        DKCHECK(!(recursive && (INCLUDE_DOT_DOT & file_type_)));
        pending_paths_.push(root_path);
    }

    DirEnumerator::DirEnumerator(const turbo::FilePath &root_path,
                                   bool recursive,
                                   int FileType,
                                   const turbo::FilePath &pattern)
            : current_directory_entry_(0),
              root_path_(root_path),
              recursive_(recursive),
              file_type_(FileType),
              pattern_(root_path / (pattern).string()) {
        // INCLUDE_DOT_DOT must not be specified if recursive.
        DKCHECK(!(recursive && (INCLUDE_DOT_DOT & file_type_)));
        // The Windows version of this code appends the pattern to the root_path,
        // potentially only matching against items in the top-most directory.
        // Do the same here.
        if (pattern.empty())
            pattern_ = "";
        pending_paths_.push(root_path);
    }

    DirEnumerator::~DirEnumerator() {
    }

    turbo::FilePath DirEnumerator::Next() {
        ++current_directory_entry_;

        // While we've exhausted the entries in the current directory, do the next
        while (current_directory_entry_ >= directory_entries_.size()) {
            if (pending_paths_.empty())
                return turbo::FilePath();

            root_path_ = pending_paths_.top();
            pending_paths_.pop();

            std::vector<FileInfo> entries;
            if (!ReadDirectory(&entries, root_path_, file_type_ & SHOW_SYM_LINKS))
                continue;

            directory_entries_.clear();
            current_directory_entry_ = 0;
            for (std::vector<FileInfo>::const_iterator i = entries.begin();
                 i != entries.end(); ++i) {
                turbo::FilePath full_path = root_path_ / (i->filename_);
                if (ShouldSkip(full_path))
                    continue;

                if (pattern_.size() &&
                    fnmatch(pattern_.c_str(), full_path.string().c_str(), FNM_NOESCAPE))
                    continue;

                if (recursive_ && S_ISDIR(i->stat_.st_mode))
                    pending_paths_.push(full_path);

                if ((S_ISDIR(i->stat_.st_mode) && (file_type_ & DIRECTORIES)) ||
                    (!S_ISDIR(i->stat_.st_mode) && (file_type_ & FILES)))
                    directory_entries_.push_back(*i);
            }
        }

        return root_path_ / (
                directory_entries_[current_directory_entry_].filename_);
    }

    DirEnumerator::FileInfo DirEnumerator::get_info() const {
        return directory_entries_[current_directory_entry_];
    }

    bool DirEnumerator::ReadDirectory(std::vector<FileInfo> *entries,
                                       const turbo::FilePath &source, bool show_links) {
        turbo::ThreadRestrictions::AssertIOAllowed();
        DIR *dir = opendir(source.string().c_str());
        if (!dir)
            return false;

#if !defined(OS_LINUX) && !defined(OS_MACOSX) && !defined(OS_BSD) && \
    !defined(OS_SOLARIS) && !defined(OS_ANDROID)
#error Port warning: depending on the definition of struct dirent, \
         additional space for pathname may be needed
#endif

        struct dirent *dent;
        // readdir_r is marked as deprecated since glibc 2.24.
        // Using readdir on _different_ DIR* object is already thread-safe in
        // most modern libc implementations.
#if defined(__GLIBC__) && \
    (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 24))
        while ((dent = readdir(dir))) {
#else
            struct dirent dent_buf;
            while (readdir_r(dir, &dent_buf, &dent) == 0 && dent) {
#endif
            FileInfo info;
            info.filename_ = turbo::FilePath(dent->d_name);

            turbo::FilePath full_name = source / (dent->d_name);
            int ret;
            if (show_links)
                ret = lstat(full_name.string().c_str(), &info.stat_);
            else
                ret = stat(full_name.string().c_str(), &info.stat_);
            if (ret < 0) {
                // Print the stat() error message unless it was ENOENT and we're
                // following symlinks.
                if (!(errno == ENOENT && !show_links)) {
                    DKLOG(ERROR) << "Couldn't stat "
                                << (source / dent->d_name).string();
                }
                memset(&info.stat_, 0, sizeof(info.stat_));
            }
            entries->push_back(info);
        }

        closedir(dir);
        return true;
    }

}  // namespace turbo
