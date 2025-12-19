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

#include <turbo/files/file.h>
#include <turbo/base/macros.h>

#if defined(OS_WIN)
#include <windows.h>
#endif

namespace turbo {

    class TURBO_EXPORT MemoryMappedFile {
     public:
      // The default constructor sets all members to invalid/null values.
      MemoryMappedFile();
      ~MemoryMappedFile();

      // Opens an existing file and maps it into memory. Access is restricted to
      // read only. If this object already points to a valid memory mapped file
      // then this method will fail and return false. If it cannot open the file,
      // the file does not exist, or the memory mapping fails, it will return false.
      // Later we may want to allow the user to specify access.
      bool initialize(const turbo::FilePath& file_name);

      // As above, but works with an already-opened file. MemoryMappedFile takes
      // ownership of |file| and closes it when done.
      bool initialize(File file);

    #if defined(OS_WIN)
      // Opens an existing file and maps it as an image section. Please refer to
      // the Initialize function above for additional information.
      bool InitializeAsImageSection(const turbo::FilePath& file_name);
    #endif  // OS_WIN

      const uint8_t* data() const { return data_; }
      size_t length() const { return length_; }

      // Is file_ a valid file handle that points to an open, memory mapped file?
      bool is_valid() const;

     private:
      // Map the file to memory, set data_ to that memory address. Return true on
      // success, false on any kind of failure. This is a helper for Initialize().
      bool MapFileToMemory();

      // Closes all open handles.
      void CloseHandles();

      File file_;
      uint8_t* data_;
      size_t length_;

    #if defined(OS_WIN)
      win::ScopedHandle file_mapping_;
      bool image_;  // Map as an image.
    #endif

      TURBO_DISALLOW_COPY_AND_ASSIGN(MemoryMappedFile);
    };

}  // namespace turbo
