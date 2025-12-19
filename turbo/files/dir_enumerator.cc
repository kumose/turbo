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

#include <turbo/files/dir_enumerator.h>

#include <turbo/files/file_util.h>

namespace turbo {

    DirEnumerator::FileInfo::~FileInfo() {
    }

    bool DirEnumerator::ShouldSkip(const turbo::FilePath &path) {
        auto basename = path.filename().string();
        return basename == "." ||
               (basename == ".." &&
                !(INCLUDE_DOT_DOT & file_type_));
    }

}  // namespace turbo
