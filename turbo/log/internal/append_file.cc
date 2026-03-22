//
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
//

#include <turbo/log/internal/append_file.h>
#include <turbo/base/internal/strerror.h>
#include <turbo/log/internal/fs_helper.h>
#include <cerrno>
#include <fstream>
#include <cstring>

namespace turbo::log_internal {

    AppendFile::~AppendFile() {
        if (_file != nullptr) {
            ::fclose(_file);
        }
    }

    int AppendFile::initialize(std::string_view path) {
        close();
        _path.assign(path.data(), path.size());
        create_dir(dir_name(_path));
        _file = fopen(_path.c_str(), "ab");
        if (_file == nullptr) {
            return errno;
        }
        _written = filesize(_file);
        ::setbuffer(_file, _buffer, sizeof(_buffer));
        return 0;
    }

    int AppendFile::reopen() {
        if (_file != nullptr) {
            ::fclose(_file);
            _file = nullptr;
        }
        create_dir(dir_name(_path));
        _file = fopen(_path.c_str(), "ab");
        if (_file == nullptr) {
            return errno;
        }
        _written = filesize(_file);
        ::setbuffer(_file, _buffer, sizeof(_buffer));
        return 0;
    }

    ssize_t AppendFile::write(std::string_view message) {
        if (_file == nullptr) {
            return -1;
        }
        ssize_t written = 0;
        auto len = message.size();
        auto logline = message.data();
        while (written != len) {
            size_t remain = len - written;
            auto n = ::fwrite_unlocked(logline, 1, remain, _file);
            if (n != remain) {
                int err = ferror(_file);
                if (err) {
                    fprintf(stderr, "AppendFile::append() failed %d\n", err);
                    break;
                }
            }
            written += n;
        }
        if(written > 0) {
            _written += written;
        }

        return written;
    }

    void AppendFile::flush() {
        if (_file == nullptr) {
            return;
        }
        ::fflush(_file);
    }

    void AppendFile::close() {
        if (_file != nullptr) {
            ::fclose(_file);
            _file = nullptr;
        }
    }


}  // namespace turbo::log_internal
