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

#include <stack>
#include <vector>
#include <turbo/base/macros.h>
#include <turbo/files/filesystem.h>
#include <turbo/times/time.h>

namespace turbo {

    class TURBO_EXPORT FileEnumerator {
    public:
        FileEnumerator(const turbo::FilePath &path);

        // test if dir is exists
        // and open dir ok
        operator bool() const {
            return _valid;
        }

        turbo::DirectoryIterator begin() const;

        turbo::DirectoryIterator end() const;

        const turbo::DirectoryEntry *next();

        // both dir and files
        std::error_code list_all(std::vector<turbo::DirectoryEntry> &result) const;

        // only files. regular file
        std::error_code list_files(std::vector<turbo::DirectoryEntry> &result, bool include_symklin = false) const;

        // only dirs.
        std::error_code list_dirs(std::vector<turbo::DirectoryEntry> &result) const;

    private:
        turbo::FilePath _path;
        std::error_code _ec;
        DirectoryIterator _itr;
        bool _valid{false};
        bool _first{true};
    };
}  // namespace turbo
