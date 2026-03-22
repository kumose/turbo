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

#include <cerrno>
#include <turbo/base/macros.h>

namespace turbo {

// Simple scoper that saves the current value of errno, resets it to 0, and on
// destruction puts the old value back.
class ScopedClearErrno {
 public:
  ScopedClearErrno() : old_errno_(errno) {
    errno = 0;
  }
  ~ScopedClearErrno() {
    if (errno == 0)
      errno = old_errno_;
  }

 private:
  const int old_errno_;

  TURBO_DISALLOW_COPY_AND_ASSIGN(ScopedClearErrno);
};

}  // namespace turbo
