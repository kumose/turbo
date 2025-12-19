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

#include <turbo/files/scoped_file.h>
#include <turbo/log/logging.h>

#if defined(OS_POSIX)
#include <unistd.h>
#include <turbo/base/internal/eintr_wrapper.h>
#endif

namespace turbo::internal {

    #if defined(OS_POSIX)

    // static
    void ScopedFDCloseTraits::Free(int fd) {
      // It's important to crash here.
      // There are security implications to not closing a file descriptor
      // properly. As file descriptors are "capabilities", keeping them open
      // would make the current process keep access to a resource. Much of
      // Chrome relies on being able to "drop" such access.
      // It's especially problematic on Linux with the setuid sandbox, where
      // a single open directory would bypass the entire security model.
      PKCHECK(0 == IGNORE_EINTR(close(fd)));
    }

    #endif  // OS_POSIX

}  // namespace turbo::internal
