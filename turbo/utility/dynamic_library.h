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
