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
