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

#ifndef __const__
#define __const__ __unused__
#endif

#include <errno.h>                           // errno
#include <turbo/base/macros.h>                     // TURBO_CONCAT

//-----------------------------------------
// Use system errno before defining yours !
//-----------------------------------------
//
// To add new errno, you shall define the errno in header first, either by
// macro or constant, or even in protobuf.
//
//     #define ESTOP -114                // C/C++
//     static const int EMYERROR = 30;   // C/C++
//     const int EMYERROR2 = -31;        // C++ only
//
// Then you can register description of the error by calling
// TURBO_REGISTER_ERRNO(the_error_number, its_description) in global scope of
// a .cpp or .cc files which will be linked.
// 
//     TURBO_REGISTER_ERRNO(ESTOP, "the thread is stopping")
//     TURBO_REGISTER_ERRNO(EMYERROR, "my error")
//
// Once the error is successfully defined:
//     km_error(error_code) returns the description.
//     km_error() returns description of last system error code.
//
// %m in printf-alike functions does NOT recognize errors defined by
// TURBO_REGISTER_ERRNO, you have to explicitly print them by %s.
//
//     errno = ESTOP;
//     printf("Something got wrong, %m\n");            // NO
//     printf("Something got wrong, %s\n", km_error());  // YES
//
// When the error number is re-defined, a linking error will be reported:
// 
//     "redefinition of `class TurboErrnoHelper<30>'"
//
// Or the program aborts at runtime before entering main():
// 
//     "Fail to define EMYERROR(30) which is already defined as `Read-only file system', abort"
//

namespace turbo {
// You should not call this function, use TURBO_REGISTER_ERRNO instead.
    extern int DescribeCustomizedErrno(int, const char *, const char *);
}

template<int error_code>
class TurboErrnoHelper {
};

class TurboErrno {
public:
    int error_code{0};
    const char *error_msg{nullptr};
};

#define TURBO_REGISTER_ERRNO(error_code, description)                   \
    const int ALLOW_UNUSED TURBO_CONCAT(kumo_errno_dummy_, __LINE__) =              \
        ::turbo::DescribeCustomizedErrno((error_code), #error_code, (description)); \
    template <> class TurboErrnoHelper<(int)(error_code)> {};

const char *km_error(int error_code);

const char *km_error();

namespace turbo {

    // `ErrnoCapture` reset the value of `errno` to `0` upon construction and restores it
    // to `0` upon deletion.  It is used in low-level code and must be super fast.  Do not
    // add instrumentation, even in debug modes.
    class ErrnoCapture {
    public:
        ErrnoCapture() {
            errno = 0;
        }

        ~ErrnoCapture() {
            errno = 0;
        }

        int capture() {
            saved_errno_ = errno;
            return saved_errno_;
        }

        int operator()() const {
            return saved_errno_;
        }

    private:
        int saved_errno_;
    };

}  // namespace turbo
