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

#include <turbo/files/scoped_temp_dir.h>

#include <turbo/files/file_util.h>
#include <turbo/log/logging.h>

namespace turbo {

    ScopedTempDir::~ScopedTempDir() {
        if (!path_.empty() && !remove().ok())
            DKLOG(WARNING) << "Could not delete temp dir in dtor.";
    }

    Status ScopedTempDir::create_unique_temp_dir() {
        if (!path_.empty()) {
            return already_exists_error("already exists: ", path_.string());
        }

        // This "scoped_dir" prefix is only used on Windows and serves as a template
        // for the unique name.
        return create_new_temp_directory(FilePath("scoped_dir")).try_value(&path_);
    }

    Status ScopedTempDir::create_unique_temp_dir_under_path(const turbo::FilePath &base_path) {
        if (!path_.empty())
            return already_exists_error("already exists: ", path_.string());

        // If |base_path| does not exist, create it.
        TURBO_RETURN_NOT_OK(turbo::create_directories(base_path));

        // Create a new, uniquely named directory under |base_path|.
        return turbo::create_temporary_dir_in_dir(base_path,
                                                  turbo::FilePath("scoped_dir_")).try_value(&path_);
    }

    Status ScopedTempDir::set(const turbo::FilePath &path) {
        if (!path_.empty())
            return already_exists_error("already exists: ", path_.string());

        TURBO_MOVE_OR_RAISE(auto et, turbo::exists(path));
        if (et) {
            TURBO_RETURN_NOT_OK(create_directories(path));
        }

        path_ = path;
        return turbo::OkStatus();
    }

    Status ScopedTempDir::remove() {
        if (path_.empty())
            return not_found_error("path empty");
        std::error_code ec;
        TURBO_MOVE_OR_RAISE(auto ret, turbo::remove_all(path_));
        if (ret) {
            // We only clear the path if deleted the directory.
            path_.clear();
        }

        return turbo::OkStatus();
    }

    turbo::FilePath ScopedTempDir::take() {
        turbo::FilePath ret = path_;
        path_ = "";
        return ret;
    }

    bool ScopedTempDir::is_valid() const {
        std::error_code ec;
        return !path_.empty() && turbo::exists(path_, ec);
    }

    Result<std::unique_ptr<ScopedTempDir>> ScopedTempDir::create(const turbo::FilePath &path) {
        std::unique_ptr<ScopedTempDir> ret = std::make_unique<ScopedTempDir>();
        auto rs = ret->create_unique_temp_dir_under_path(path);
        if(!rs.ok()) {
            return rs;
        }
        return ret;
    }

}  // namespace turbo
