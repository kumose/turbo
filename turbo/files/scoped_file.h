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

#include <stdio.h>


#include <turbo/log/logging.h>
#include <turbo/memory/scoped_ptr.h>
#include <turbo/base/scoped_generic.h>
#include <turbo/base/macros.h>

namespace turbo {

    namespace internal {

#if defined(OS_POSIX)

        struct TURBO_EXPORT ScopedFDCloseTraits {
            static int InvalidValue() {
                return -1;
            }

            static void Free(int fd);
        };

#endif

    }  // namespace internal

// -----------------------------------------------------------------------------

#if defined(OS_POSIX)
// A low-level Posix file descriptor closer class. Use this when writing
// platform-specific code, especially that does non-file-like things with the
// FD (like sockets).
//
//
// If you're writing cross-platform code that deals with actual files, you
// should generally use turbo::File instead which can be constructed with a
// handle, and in addition to handling ownership, has convenient cross-platform
// file manipulation functions on it.
    typedef ScopedGeneric<int, internal::ScopedFDCloseTraits> ScopedFD;
#endif

//// Automatically closes |FILE*|s.
    class ScopedFILE {
    private:
        struct RValue {
            explicit RValue(ScopedFILE *object) : object(object) {}

            ScopedFILE *object;
        };

        ScopedFILE(ScopedFILE &) = delete;

        ScopedFILE &operator=(ScopedFILE &) = delete;

    public:
        operator RValue() { return RValue(this); }

        ScopedFILE Pass() { return ScopedFILE(RValue(this)); }

    public:
        ScopedFILE() : _fp(nullptr) {}

        // Open file at |path| with |mode|.
        // If fopen failed, operator FILE* returns nullptr and errno is set.
        ScopedFILE(const char *path, const char *mode) {
            _fp = fopen(path, mode);
        }

        // |fp| must be the return value of fopen as we use fclose in the
        // destructor, otherwise the behavior is undefined
        explicit ScopedFILE(FILE *fp) : _fp(fp) {}

        ScopedFILE(RValue rvalue) {
            _fp = rvalue.object->_fp;
            rvalue.object->_fp = nullptr;
        }

        ~ScopedFILE() {
            if (_fp != nullptr) {
                fclose(_fp);
                _fp = nullptr;
            }
        }

        // Close current opened file and open another file at |path| with |mode|
        void reset(const char *path, const char *mode) {
            reset(fopen(path, mode));
        }

        void reset() { reset(nullptr); }

        void reset(FILE *fp) {
            if (_fp != nullptr) {
                fclose(_fp);
                _fp = nullptr;
            }
            _fp = fp;
        }

        // Set internal FILE* to nullptr and return previous value.
        FILE *release() {
            FILE *const prev_fp = _fp;
            _fp = nullptr;
            return prev_fp;
        }

        operator FILE *() const { return _fp; }

        FILE *get() { return _fp; }

    private:
        FILE *_fp;
    };

}  // namespace turbo
