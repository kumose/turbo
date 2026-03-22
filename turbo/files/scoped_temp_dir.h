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

// An object representing a temporary / scratch directory that should be cleaned
// up (recursively) when this object goes out of scope.  Note that since
// deletion occurs during the destructor, no further error handling is possible
// if the directory fails to be deleted.  As a result, deletion is not
// guaranteed by this class.
//
// Multiple calls to the methods which establish a temporary directory
// (create_unique_temp_dir, create_unique_temp_dir_under_path, and Set) must have
// intervening calls to Delete or Take, or the calls will fail.

#include <turbo/base/macros.h>
#include <turbo/files/filesystem.h>

namespace turbo {

    class TURBO_EXPORT ScopedTempDir {
    public:
        // No directory is owned/created initially.
        ScopedTempDir() = default;

        // Recursively delete path.
        ~ScopedTempDir();

        ScopedTempDir(const ScopedTempDir &) = delete;

        ScopedTempDir &operator=(const ScopedTempDir &) = delete;

        // Creates a unique directory in TempPath, and takes ownership of it.
        // See file_util::CreateNewTemporaryDirectory.
        Status create_unique_temp_dir();

        // Creates a unique directory under a given path, and takes ownership of it.
        Status create_unique_temp_dir_under_path(const turbo::FilePath &path);

        // Takes ownership of directory at |path|, creating it if necessary.
        // Don't call multiple times unless Take() has been called first.
        Status set(const turbo::FilePath &path);

        // Deletes the temporary directory wrapped by this object.
        Status remove();

        // Caller takes ownership of the temporary directory so it won't be destroyed
        // when this object goes out of scope.
        turbo::FilePath take();

        const turbo::FilePath &path() const { return path_; }

        // Returns true if path_ is non-empty and exists.
        bool is_valid() const;

        static Result<std::unique_ptr<ScopedTempDir>> create(const turbo::FilePath &path);

    private:
        turbo::FilePath path_;
    };

}  // namespace turbo
