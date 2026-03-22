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
