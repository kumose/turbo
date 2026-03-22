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
//
// -----------------------------------------------------------------------------
// File: clock.h
// -----------------------------------------------------------------------------
//
// This header file contains utility functions for working with the system-wide
// realtime clock. For descriptions of the main time abstractions used within
// this header file, consult the time.h header file.

#pragma once

#include <cstdint>
#include <turbo/base/macros.h>
#include <turbo/times/time.h>

namespace turbo {

// GetCurrentTimeNanos()
//
// Returns the current time, expressed as a count of nanoseconds since the Unix
// Epoch (https://en.wikipedia.org/wiki/Unix_time). Prefer `turbo::Time::current_time()` instead
// for all but the most performance-sensitive cases (i.e. when you are calling
// this function hundreds of thousands of times per second).
int64_t GetCurrentTimeNanos();

}  // namespace turbo
