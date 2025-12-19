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
