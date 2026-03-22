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

#include <stack>
#include <vector>
#include <turbo/base/macros.h>
#include <turbo/files/filesystem.h>
#include <turbo/times/time.h>

#if defined(OS_WIN)
#include <windows.h>
#elif defined(OS_POSIX)
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace turbo {

    // A class for enumerating the files in a provided path. The order of the
    // results is not guaranteed.
    //
    // This is blocking. Do not use on critical threads.
    //
    // Example:
    //
    //   turbo::DirEnumerator enum(my_dir, false, turbo::DirEnumerator::FILES,
    //                             FILE_PATH_LITERAL("*.txt"));
    //   for (turbo::FilePath name = enum.Next(); !name.empty(); name = enum.Next())
    //     ...
    class TURBO_EXPORT DirEnumerator {
     public:
      // Note: copy & assign supported.
      class TURBO_EXPORT FileInfo {
       public:
        FileInfo();
        ~FileInfo();

        bool IsDirectory() const;

        // The name of the file. This will not include any path information. This
        // is in constrast to the value returned by DirEnumerator.Next() which
        // includes the |root_path| passed into the DirEnumerator constructor.
        turbo::FilePath GetName() const;

        int64_t GetSize() const;
        turbo::Time GetLastModifiedTime() const;

    #if defined(OS_WIN)
        // Note that the cAlternateFileName (used to hold the "short" 8.3 name)
        // of the WIN32_FIND_DATA will be empty. Since we don't use short file
        // names, we tell Windows to omit it which speeds up the query slightly.
        const WIN32_FIND_DATA& find_data() const { return find_data_; }
    #elif defined(OS_POSIX)
        const struct stat& stat() const { return stat_; }
    #endif

       private:
        friend class DirEnumerator;

    #if defined(OS_WIN)
        WIN32_FIND_DATA find_data_;
    #elif defined(OS_POSIX)
        struct stat stat_;
          turbo::FilePath filename_;
    #endif
      };

      enum ListFileType {
        FILES                 = 1 << 0,
        DIRECTORIES           = 1 << 1,
        INCLUDE_DOT_DOT       = 1 << 2,
    #if defined(OS_POSIX)
        SHOW_SYM_LINKS        = 1 << 4,
    #endif
      };

      // |root_path| is the starting directory to search for. It may or may not end
      // in a slash.
      //
      // If |recursive| is true, this will enumerate all matches in any
      // subdirectories matched as well. It does a breadth-first search, so all
      // files in one directory will be returned before any files in a
      // subdirectory.
      //
      // |FileType|, a bit mask of ListFileType, specifies whether the enumerator
      // should match files, directories, or both.
      //
      // |pattern| is an optional pattern for which files to match. This
      // works like shell globbing. For example, "*.txt" or "Foo???.doc".
      // However, be careful in specifying patterns that aren't cross platform
      // since the underlying code uses OS-specific matching routines.  In general,
      // Windows matching is less featureful than others, so test there first.
      // If unspecified, this will match all files.
      // NOTE: the pattern only matches the contents of root_path, not files in
      // recursive subdirectories.
      // TODO(erikkay): Fix the pattern matching to work at all levels.
      DirEnumerator(const turbo::FilePath& root_path,
                     bool recursive,
                     int FileType);
      DirEnumerator(const turbo::FilePath& root_path,
                     bool recursive,
                     int FileType,
                     const turbo::FilePath& pattern);
      ~DirEnumerator();

      // Returns the next file or an empty string if there are no more results.
      //
      // The returned path will incorporate the |root_path| passed in the
      // constructor: "<root_path>/file_name.txt". If the |root_path| is absolute,
      // then so will be the result of Next().
      turbo::FilePath Next();

      // Write the file info into |info|.
      FileInfo get_info() const;

     private:
      // Returns true if the given path should be skipped in enumeration.
      bool ShouldSkip(const turbo::FilePath& path);

    #if defined(OS_WIN)
      // True when find_data_ is valid.
      bool has_find_data_;
      WIN32_FIND_DATA find_data_;
      HANDLE find_handle_;
    #elif defined(OS_POSIX)

      // Read the filenames in source into the vector of DirectoryEntryInfo's
      static bool ReadDirectory(std::vector<FileInfo>* entries,
                                const turbo::FilePath& source, bool show_links);

      // The files in the current directory
      std::vector<FileInfo> directory_entries_;

      // The next entry to use from the directory_entries_ vector
      size_t current_directory_entry_;
    #endif

        turbo::FilePath root_path_;
      bool recursive_;
      int file_type_;
      std::string pattern_;  // Empty when we want to find everything.

      // A stack that keeps track of which subdirectories we still need to
      // enumerate in the breadth-first search.
      std::stack<turbo::FilePath> pending_paths_;

      TURBO_DISALLOW_COPY_AND_ASSIGN(DirEnumerator);
    };

}  // namespace turbo
