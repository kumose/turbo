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

#include <string>
#include <turbo/base/macros.h>

namespace turbo {

// --------------------------------------------------------------------------
// systems and are not proper for performance critical situations.
// For fast random numbers, check fast_rand.h
// --------------------------------------------------------------------------

// Returns a random number in range [0, kuint64max]. Thread-safe.
TURBO_EXPORT uint64_t RandUint64();

// Returns a random number between min and max (inclusive). Thread-safe.
TURBO_EXPORT int RandInt(int min, int max);

// Returns a random number in range [0, range).  Thread-safe.
//
// Note that this can be used as an adapter for std::random_shuffle():
// Given a pre-populated |std::vector<int> myvector|, shuffle it as
//   std::random_shuffle(myvector.begin(), myvector.end(), turbo::RandGenerator);
TURBO_EXPORT uint64_t RandGenerator(uint64_t range);

// Returns a random double in range [0, 1). Thread-safe.
TURBO_EXPORT double RandDouble();

// Given input |bits|, convert with maximum precision to a double in
// the range [0, 1). Thread-safe.
TURBO_EXPORT double BitsToOpenEndedUnitInterval(uint64_t bits);

// Fills |output_length| bytes of |output| with random data.
//
// WARNING:
// Do not use for security-sensitive purposes.
// See crypto/ for cryptographically secure random number generation APIs.
TURBO_EXPORT void RandBytes(void* output, size_t output_length);

// Fills a string of length |length| with random data and returns it.
// |length| should be nonzero.
//
// Note that this is a variation of |RandBytes| with a different return type.
// The returned string is likely not ASCII/UTF-8. Use with care.
//
// WARNING:
// Do not use for security-sensitive purposes.
// See crypto/ for cryptographically secure random number generation APIs.
TURBO_EXPORT std::string RandBytesAsString(size_t length);

#if defined(OS_POSIX)
TURBO_EXPORT int GetUrandomFD();
#endif

}  // namespace turbo
