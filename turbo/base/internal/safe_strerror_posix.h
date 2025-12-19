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

#include <string>
#include <turbo/base/macros.h>


// BEFORE using anything from this file, first look at PKLOG and friends in
// logging.h and use them instead if applicable.
//
// This file declares safe, portable alternatives to the POSIX strerror()
// function. strerror() is inherently unsafe in multi-threaded apps and should
// never be used. Doing so can cause crashes. Additionally, the thread-safe
// alternative strerror_r varies in semantics across platforms. Use these
// functions instead.

// Thread-safe strerror function with dependable semantics that never fails.
// It will write the string form of error "err" to buffer buf of length len.
// If there is an error calling the OS's strerror_r() function then a message to
// that effect will be printed into buf, truncating if necessary. The final
// result is always null-terminated. The value of errno is never changed.
//
// Use this instead of strerror_r().
TURBO_EXPORT void safe_strerror_r(int err, char *buf, size_t len);

// Calls safe_strerror_r with a buffer of suitable size and returns the result
// in a C++ string.
//
// Use this instead of strerror(). Note though that safe_strerror_r will be
// more robust in the case of heap corruption errors, since it doesn't need to
// allocate a string.
TURBO_EXPORT std::string safe_strerror(int err);
