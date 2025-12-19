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
#pragma once

#include <turbo/base/macros.h>
#include <turbo/utility/status.h>
#if defined(OS_WIN)
#include <windows.h>
#elif defined(OS_POSIX)

#include <sys/stat.h>
#include <unistd.h>

#endif

#include <stdio.h>
#include <turbo/files/file.h>
#include <set>
#include <string>
#include <vector>
#include <turbo/memory/scoped_ptr.h>
#include <turbo/files/filesystem.h>
#include <turbo/files/io_util.h>
#include <turbo/files/reader.h>
#include <turbo/files/writer.h>

#if defined(OS_POSIX)
#include <turbo/log/logging.h>
#include <turbo/base/internal/eintr_wrapper.h>
#endif

namespace turbo {

    // Returns the total number of bytes used by all the files under |root_path|.
    // If the path does not exist the function returns 0.
    //
    // This function is implemented using the DirEnumerator class so it is not
    // particularly speedy in any platform.
    TURBO_EXPORT Result<int64_t> compute_directory_size(const turbo::FilePath &root_path);

    // Returns true if the contents of the two files given are equal, false
    // otherwise.  If either file can't be read, returns false.
    TURBO_EXPORT bool contents_equal(const turbo::FilePath &filename1,
                                     const turbo::FilePath &filename2);

    // Returns true if the contents of the two text files given are equal, false
    // otherwise.  This routine treats "\r\n" and "\n" as equivalent.
    TURBO_EXPORT bool text_contents_equal(const turbo::FilePath &filename1,
                                          const turbo::FilePath &filename2);


    // Create a directory within another directory.
    // Extra characters will be appended to |prefix| to ensure that the
    // new directory does not have the same name as an existing directory.
    TURBO_EXPORT turbo::Result<turbo::FilePath> create_temporary_dir_in_dir(const turbo::FilePath &base_dir,
                                                  const turbo::FilePath &prefix);

    // Sets the time of the last access and the time of the last modification.
    TURBO_EXPORT bool touch_file(const turbo::FilePath &path,
                                 const turbo::Time &last_accessed,
                                 const turbo::Time &last_modified);

    // Wrapper for fopen-like calls. Returns non-nullptr FILE* on success.
    TURBO_EXPORT turbo::Result<FILE*> open_file(const turbo::FilePath &filename, const char *mode);

    // Closes file opened by open_file. Returns true on success.
    TURBO_EXPORT turbo::Status close_file(FILE *file);

    // Associates a standard FILE stream with an existing File. Note that this
    // functions take ownership of the existing File.
    TURBO_EXPORT FILE *file_to_FILE(File file, const char *mode);

    // Truncates an open file to end at the location of the current file pointer.
    // This is a cross-platform analog to Windows' SetEndOfFile() function.
    TURBO_EXPORT bool truncate_file(FILE *file);

    turbo::Result<turbo::FilePath> create_new_temp_directory(const turbo::FilePath &prefix);

    // Gets the current working directory for the process.
    TURBO_EXPORT bool get_current_directory(turbo::FilePath *path);

    // Sets the current working directory for the process.
    TURBO_EXPORT bool set_current_directory(const turbo::FilePath &path);


}  // namespace turbo

// -----------------------------------------------------------------------------

namespace file_util {

    // Functor for |ScopedFILE| (below).
    struct ScopedFPClose {
        inline void operator()(FILE *x) const {
            if (x)
                fclose(x);
        }
    };

    // Automatically closes |FILE*|s.
    typedef turbo::scoped_ptr<FILE, ScopedFPClose> ScopedFP;

}  // namespace file_util

