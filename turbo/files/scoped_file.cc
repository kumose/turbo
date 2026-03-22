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
