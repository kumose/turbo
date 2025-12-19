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

#include <turbo/files/file_enumerator.h>

namespace turbo {

    FileEnumerator::FileEnumerator(const turbo::FilePath &path) : _path(path) {
        DirectoryIterator itr(_path, _ec);
        _valid = _ec ? false : true;
    }

    DirectoryIterator FileEnumerator::begin() const {
        std::error_code ec;
        DirectoryIterator itr(_path, ec);
        if (ec) {
            return {};
        }
        return itr;
    }

    DirectoryIterator FileEnumerator::end() const {
        return {};
    }

    const DirectoryEntry *FileEnumerator::next() {
        if (_first && !_ec) {
            _itr = DirectoryIterator(_path, _ec);
            _first = false;
            if (_ec) {
                return nullptr;
            }
        } else if(_itr != DirectoryIterator()){
            ++_itr;
        } else {
            return nullptr;
        }

        return _itr.operator->();
    }

    std::error_code FileEnumerator::list_all(std::vector<DirectoryEntry> &result) const {
        result.clear();
        std::error_code ec;
        DirectoryIterator itr(_path, ec);
        if (ec) {
            return ec;
        }
        while (!ec && itr != DirectoryIterator()) {
            result.push_back(*itr);
            itr.increment(ec);
        }
        return ec;
    }

    std::error_code FileEnumerator::list_files(std::vector<DirectoryEntry> &result, bool include_symklin) const {
        result.clear();
        std::error_code ec;
        DirectoryIterator itr(_path, ec);
        if (ec) {
            return ec;
        }
        while (!ec && itr != DirectoryIterator()) {
            if (itr->is_regular_file(ec) || (include_symklin && itr->is_symlink(ec))) {
                result.push_back(*itr);
            }
            itr.increment(ec);
        }
        return ec;
    }

    // only dirs.
    std::error_code FileEnumerator::list_dirs(std::vector<DirectoryEntry> &result) const {
        result.clear();
        std::error_code ec;
        DirectoryIterator itr(_path, ec);
        if (ec) {
            return ec;
        }
        while (!ec && itr != DirectoryIterator()) {
            if (itr->is_directory(ec)) {
                result.push_back(*itr);
            }
            itr.increment(ec);
        }
        return ec;
    }
}  // namespace turbo
