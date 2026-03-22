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
#include <turbo/files/file_util.h>
#include <turbo/threading/thread_restrictions.h>
#if defined(OS_WIN)
#include <io.h>
#endif

#include <stdio.h>
#include <fstream>
#include <limits>
#include <turbo/log/logging.h>


namespace turbo {

    namespace {

        // The maximum number of 'uniquified' files we will try to create.
        // This is used when the filename we're trying to download is already in use,
        // so we create a new unique filename by appending " (nnn)" before the
        // extension, where 1 <= nnn <= kMaxUniqueFiles.
        // Also used by code that cleans up said files.
        static const int kMaxUniqueFiles = 100;

    }  // namespace

    Result<int64_t> compute_directory_size(const turbo::FilePath &root_path) {
        int64_t running_size = 0;
        TURBO_MOVE_OR_RAISE(turbo::DirectoryIterator begin, turbo::DirectoryIterator::create(root_path));
        turbo::DirectoryIterator end;
        std::error_code ec;
        while (begin != end) {
            if (begin->is_regular_file(ec)) {
                TURBO_MOVE_OR_RAISE(auto sz, begin->file_size());
                running_size += sz;
            }
        }
        return running_size;
    }


    bool contents_equal(const turbo::FilePath &filename1, const turbo::FilePath &filename2) {
        // We open the file in binary format even if they are text files because
        // we are just comparing that bytes are exactly same in both files and not
        // doing anything smart with text formatting.
        std::ifstream file1(filename1.string().c_str(),
                            std::ios::in | std::ios::binary);
        std::ifstream file2(filename2.string().c_str(),
                            std::ios::in | std::ios::binary);

        // Even if both files aren't openable (and thus, in some sense, "equal"),
        // any unusable file yields a result of "false".
        if (!file1.is_open() || !file2.is_open())
            return false;

        const int BUFFER_SIZE = 2056;
        char buffer1[BUFFER_SIZE], buffer2[BUFFER_SIZE];
        do {
            file1.read(buffer1, BUFFER_SIZE);
            file2.read(buffer2, BUFFER_SIZE);

            if ((file1.eof() != file2.eof()) ||
                (file1.gcount() != file2.gcount()) ||
                (memcmp(buffer1, buffer2, file1.gcount()))) {
                file1.close();
                file2.close();
                return false;
            }
        } while (!file1.eof() || !file2.eof());

        file1.close();
        file2.close();
        return true;
    }

    bool text_contents_equal(const turbo::FilePath &filename1, const turbo::FilePath &filename2) {
        std::ifstream file1(filename1.string().c_str(), std::ios::in);
        std::ifstream file2(filename2.string().c_str(), std::ios::in);

        // Even if both files aren't openable (and thus, in some sense, "equal"),
        // any unusable file yields a result of "false".
        if (!file1.is_open() || !file2.is_open())
            return false;

        do {
            std::string line1, line2;
            getline(file1, line1);
            getline(file2, line2);

            // Check for mismatched EOF states, or any error state.
            if ((file1.eof() != file2.eof()) ||
                file1.bad() || file2.bad()) {
                return false;
            }

            // Trim all '\r' and '\n' characters from the end of the line.
            std::string::size_type end1 = line1.find_last_not_of("\r\n");
            if (end1 == std::string::npos)
                line1.clear();
            else if (end1 + 1 < line1.length())
                line1.erase(end1 + 1);

            std::string::size_type end2 = line2.find_last_not_of("\r\n");
            if (end2 == std::string::npos)
                line2.clear();
            else if (end2 + 1 < line2.length())
                line2.erase(end2 + 1);

            if (line1 != line2)
                return false;
        } while (!file1.eof() || !file2.eof());

        return true;
    }

    bool touch_file(const turbo::FilePath &path,
                    const turbo::Time &last_accessed,
                    const turbo::Time &last_modified) {
        int flags = File::FLAG_OPEN | File::FLAG_WRITE_ATTRIBUTES;

#if defined(OS_WIN)
        // On Windows, FILE_FLAG_BACKUP_SEMANTICS is needed to open a directory.
        if (DirectoryExists(path))
          flags |= File::FLAG_BACKUP_SEMANTICS;
#endif  // OS_WIN

        File file(path, flags);
        if (!file.is_valid())
            return false;

        return file.set_times(last_accessed, last_modified);
    }

    turbo::Result<FILE*> open_file(const turbo::FilePath &filename, const char *mode) {
        ThreadRestrictions::AssertIOAllowed();
        FILE *result = nullptr;
        do {
            result = fopen(filename.string().c_str(), mode);
        } while (!result && errno == EINTR);
        if(result == nullptr) {
            return turbo::errno_to_status(errno, "open file fail");
        }
        return result;
    }

    turbo::Status close_file(FILE *file) {
        if (file == nullptr)
            return turbo::OkStatus();
        if(fclose(file) != 0) {
            return turbo::errno_to_status(errno, "close fail");
        }
        return turbo::OkStatus();
    }

    bool truncate_file(FILE *file) {
        if (file == nullptr)
            return false;
        long current_offset = ftell(file);
        if (current_offset == -1)
            return false;
#if defined(OS_WIN)
        int fd = _fileno(file);
        if (_chsize(fd, current_offset) != 0)
          return false;
#else
        int fd = fileno(file);
        if (ftruncate(fd, current_offset) != 0)
            return false;
#endif
        return true;
    }


}  // namespace turbo
