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

#include <turbo/base/macros.h>
#include <turbo/files/filesystem.h>
#include <turbo/utility/status.h>

namespace turbo {

    // Append the given buffer into the file. Returns the number of bytes written,
    // or -1 on error.
    TURBO_EXPORT turbo::Status write_to_file(const turbo::FilePath &filename,
                                    const uint8_t *data, size_t size);

    TURBO_EXPORT turbo::Status write_to_file(const turbo::FilePath &filename, std::string_view data);

}  // namespace turbo
