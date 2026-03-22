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
