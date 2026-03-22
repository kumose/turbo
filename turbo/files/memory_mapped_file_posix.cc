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
