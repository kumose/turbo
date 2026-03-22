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

namespace turbo {

    File::Info::Info()
            : size(0),
              is_directory(false),
              is_symbolic_link(false) {
    }

    File::Info::~Info() {
    }

    File::File()
            : error_details_(FILE_ERROR_FAILED),
              created_(false),
              async_(false) {
    }

#if !defined(OS_NACL)

    File::File(const turbo::FilePath &name, uint32_t flags)
            : error_details_(FILE_OK),
              created_(false),
              async_(false) {
        initialize(name, flags);
    }

#endif

    File::File(PlatformFile platform_file)
            : file_(platform_file),
              error_details_(FILE_OK),
              created_(false),
              async_(false) {
#if defined(OS_POSIX)
        DKCHECK_GE(platform_file, -1);
#endif
    }

    File::File(Error error_details)
            : error_details_(error_details),
              created_(false),
              async_(false) {
    }

    File::File(RValue other)
            : file_(other.object->take_platform_file()),
              error_details_(other.object->error_details()),
              created_(other.object->created()),
              async_(other.object->async_) {
    }

    File::~File() {
        // Go through the AssertIOAllowed logic.
        close();
    }

    File &File::operator=(RValue other) {
        if (this != other.object) {
            close();
            SetPlatformFile(other.object->take_platform_file());
            error_details_ = other.object->error_details();
            created_ = other.object->created();
            async_ = other.object->async_;
        }
        return *this;
    }

#if !defined(OS_NACL)

    void File::initialize(const turbo::FilePath &name, uint32_t flags) {
        auto r = name.lexically_normal();
        if (name != r) {
            error_details_ = FILE_ERROR_ACCESS_DENIED;
            return;
        }
        initialize_unsafe(name, flags);
    }

#endif

    std::string File::ErrorToString(Error error) {
        switch (error) {
            case FILE_OK:
                return "FILE_OK";
            case FILE_ERROR_FAILED:
                return "FILE_ERROR_FAILED";
            case FILE_ERROR_IN_USE:
                return "FILE_ERROR_IN_USE";
            case FILE_ERROR_EXISTS:
                return "FILE_ERROR_EXISTS";
            case FILE_ERROR_NOT_FOUND:
                return "FILE_ERROR_NOT_FOUND";
            case FILE_ERROR_ACCESS_DENIED:
                return "FILE_ERROR_ACCESS_DENIED";
            case FILE_ERROR_TOO_MANY_OPENED:
                return "FILE_ERROR_TOO_MANY_OPENED";
            case FILE_ERROR_NO_MEMORY:
                return "FILE_ERROR_NO_MEMORY";
            case FILE_ERROR_NO_SPACE:
                return "FILE_ERROR_NO_SPACE";
            case FILE_ERROR_NOT_A_DIRECTORY:
                return "FILE_ERROR_NOT_A_DIRECTORY";
            case FILE_ERROR_INVALID_OPERATION:
                return "FILE_ERROR_INVALID_OPERATION";
            case FILE_ERROR_SECURITY:
                return "FILE_ERROR_SECURITY";
            case FILE_ERROR_ABORT:
                return "FILE_ERROR_ABORT";
            case FILE_ERROR_NOT_A_FILE:
                return "FILE_ERROR_NOT_A_FILE";
            case FILE_ERROR_NOT_EMPTY:
                return "FILE_ERROR_NOT_EMPTY";
            case FILE_ERROR_INVALID_URL:
                return "FILE_ERROR_INVALID_URL";
            case FILE_ERROR_IO:
                return "FILE_ERROR_IO";
            case FILE_ERROR_MAX:
                break;
        }

        KLOG(FATAL);
        return "";
    }

}  // namespace turbo
