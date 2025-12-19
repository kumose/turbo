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
