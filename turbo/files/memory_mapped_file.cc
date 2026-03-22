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
#include <turbo/log/logging.h>

namespace turbo {

    MemoryMappedFile::~MemoryMappedFile() {
      CloseHandles();
    }

    bool MemoryMappedFile::initialize(const turbo::FilePath& file_name) {
      if (is_valid())
        return false;

      file_.initialize(file_name, File::FLAG_OPEN | File::FLAG_READ);

      if (!file_.is_valid()) {
        DKLOG(ERROR) << "Couldn't open " << file_name;
        return false;
      }

      if (!MapFileToMemory()) {
        CloseHandles();
        return false;
      }

      return true;
    }

    bool MemoryMappedFile::initialize(File file) {
      if (is_valid())
        return false;

      file_ = file.Pass();

      if (!MapFileToMemory()) {
        CloseHandles();
        return false;
      }

      return true;
    }

    bool MemoryMappedFile::is_valid() const {
      return data_ != nullptr;
    }

}  // namespace turbo
