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


#include <turbo/base/checked_cast.h>
#include <turbo/utility/dynamic_library.h>
#include <turbo/log/logging.h>


#ifdef _WIN32
#include <Windows.h>
#else

#include <dlfcn.h>

#endif

#include <turbo/files/file_util.h>

namespace turbo {

    turbo::Result<void *> load_dynamic_library(const char *path) {
#ifdef _WIN32
        TURBO_MOVE_OR_RAISE(auto platform_path, PlatformFilename::FromString(path));
        return load_dynamic_library(platform_path);
#else
        constexpr int kFlags =
                // All undefined symbols in the shared object are resolved before dlopen() returns.
                RTLD_NOW
                // Symbols defined in  this  shared  object are not made available to
                // resolve references in subsequently loaded shared objects.
                | RTLD_LOCAL;
        if (void *handle = dlopen(path, kFlags)) return handle;
        // dlopen(3) man page: "If dlopen() fails for any reason, it returns NULL."
        // There is no null-returning non-error condition.
        auto *error = dlerror();
        return turbo::io_error("dlopen(", path, ") failed: ", error ? error : "unknown error");
#endif
    }

    turbo::Result<void *> load_dynamic_library(const turbo::FilePath &path) {
#ifdef _WIN32
        if (void* handle = LoadLibraryW(path.ToNative().c_str())) {
          return handle;
        }
        // win32 api doc: "If the function fails, the return value is NULL."
        // There is no null-returning non-error condition.
        return turbo::io_error(GetLastError(), "LoadLibrary(", path.to_string(), ") failed");
#else
        return load_dynamic_library(path.string().c_str());
#endif
    }

    turbo::Result<void *> get_symbol(void *handle, const char *name) {
        if (handle == nullptr) {
            return turbo::invalid_argument_error("Attempting to retrieve symbol '", name,
                                          "' from null library handle");
        }
#ifdef _WIN32
        if (void* sym = reinterpret_cast<void*>(
                GetProcAddress(reinterpret_cast<HMODULE>(handle), name))) {
          return sym;
        }
        // win32 api doc: "If the function fails, the return value is NULL."
        // There is no null-returning non-error condition.
        return turbo::io_error(GetLastError(), "GetProcAddress(", name, ") failed.");
#else
        if (void *sym = dlsym(handle, name)) return sym;
        // dlsym(3) man page: "On failure, they return NULL"
        // There is no null-returning non-error condition.
        auto *error = dlerror();
        return turbo::io_error("dlsym(", name, ") failed: ", error ? error : "unknown error");
#endif
    }

}  // namespace turbo
