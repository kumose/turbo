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

#include <turbo/files/memory_mapped_file.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <turbo/log/logging.h>
#include <turbo/threading/thread_restrictions.h>

namespace turbo {

    MemoryMappedFile::MemoryMappedFile() : data_(nullptr), length_(0) {
    }

    bool MemoryMappedFile::MapFileToMemory() {
        ThreadRestrictions::AssertIOAllowed();

        struct stat file_stat;
        if (fstat(file_.get_platform_file(), &file_stat) == -1) {
            DKLOG(ERROR) << "fstat " << file_.get_platform_file();
            return false;
        }
        length_ = file_stat.st_size;

        data_ = static_cast<uint8_t *>(
                mmap(nullptr, length_, PROT_READ, MAP_SHARED, file_.get_platform_file(), 0));
        if (data_ == MAP_FAILED)
            DKLOG(ERROR) << "mmap " << file_.get_platform_file();

        return data_ != MAP_FAILED;
    }

    void MemoryMappedFile::CloseHandles() {
        ThreadRestrictions::AssertIOAllowed();

        if (data_ != nullptr)
            munmap(data_, length_);
        file_.close();

        data_ = nullptr;
        length_ = 0;
    }

}  // namespace turbo
