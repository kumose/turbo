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
