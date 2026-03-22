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
#include <turbo/files/writer.h>
#include <turbo/files/io_util.h>
#include <turbo/threading/thread_restrictions.h>

namespace turbo {

    turbo::Status write_to_file(const turbo::FilePath &filename, const uint8_t *data, size_t size) {
        ThreadRestrictions::AssertIOAllowed();
        TURBO_MOVE_OR_RAISE(auto fd, file_open_writable(filename));
        return file_write(fd.fd(), data, size);
    }

    turbo::Status write_to_file(const turbo::FilePath &filename, std::string_view data) {
        return write_to_file(filename, reinterpret_cast<const uint8_t *>(data.data()), data.size());
    }

}  // namespace turbo
