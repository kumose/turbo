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
