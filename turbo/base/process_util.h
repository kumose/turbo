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

#include <sys/types.h>

namespace turbo {

// Read command line of this program. If `with_args' is true, args are
// included and separated with spaces.
// Returns length of the command line on sucess, -1 otherwise.
// NOTE: `buf' does not end with zero.
ssize_t ReadCommandLine(char* buf, size_t len, bool with_args);

// Get absolute path of this program.
// Returns length of the absolute path on sucess, -1 otherwise.
// NOTE: `buf' does not end with zero.
ssize_t GetProcessAbsolutePath(char* buf, size_t len);

} // namespace turbo
