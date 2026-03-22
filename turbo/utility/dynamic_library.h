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

#include <atomic>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <turbo/utility/status.h>
#include <turbo/base/macros.h>
#include <turbo/files/filesystem.h>

namespace turbo {


    /// \brief Load a dynamic library
    ///
    /// This wraps dlopen() except on Windows, where LoadLibrary() is called.
    /// These two platforms handle absolute paths consistently; relative paths
    /// or the library's bare name may be handled but inconsistently.
    ///
    /// \return An opaque handle for the dynamic library, which can be used for
    ///         subsequent symbol lookup. Nullptr will never be returned; instead
    ///         an error will be raised.
    TURBO_EXPORT turbo::Result<void*> load_dynamic_library(const turbo::FilePath& path);

    /// \brief Load a dynamic library
    ///
    /// An overload taking null terminated string.
    TURBO_EXPORT turbo::Result<void*> load_dynamic_library(const char* path);

    /// \brief Retrieve a symbol by name from a library handle.
    ///
    /// This wraps dlsym() except on Windows, where GetProcAddress() is called.
    ///
    /// \return The address associated with the named symbol. Nullptr will never be
    ///         returned; instead an error will be raised.
    TURBO_EXPORT turbo::Result<void*> get_symbol(void* handle, const char* name);

    template <typename T>
    turbo::Result<T*> get_symbol_as(void* handle, const char* name) {
      TURBO_MOVE_OR_RAISE(void* sym, get_symbol(handle, name));
      return reinterpret_cast<T*>(sym);
    }

}  // namespace turbo
