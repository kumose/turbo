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
#include <turbo/utility/status.h>  // errno

#if defined(OS_POSIX)

#include <cerrno>

#if defined(NDEBUG)

#define HANDLE_EINTR(x) ({ \
  decltype(x) eintr_wrapper_result; \
  do { \
    eintr_wrapper_result = (x); \
  } while (eintr_wrapper_result == -1 && errno == EINTR); \
  eintr_wrapper_result; \
})

#else

#define HANDLE_EINTR(x) ({ \
  int eintr_wrapper_counter = 0; \
  decltype(x) eintr_wrapper_result; \
  do { \
    eintr_wrapper_result = (x); \
  } while (eintr_wrapper_result == -1 && errno == EINTR && \
           eintr_wrapper_counter++ < 100); \
  eintr_wrapper_result; \
})

#endif  // NDEBUG

#define IGNORE_EINTR(x) ({ \
  decltype(x) eintr_wrapper_result;     \
  do { \
    eintr_wrapper_result = (x); \
    if (eintr_wrapper_result == -1 && errno == EINTR) { \
      eintr_wrapper_result = 0; \
    } \
  } while (0); \
  eintr_wrapper_result; \
})

#else

#define HANDLE_EINTR(x) (x)
#define IGNORE_EINTR(x) (x)

#endif  // OS_POSIX
