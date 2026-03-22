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
// File: log/die_if_null.h
// -----------------------------------------------------------------------------
//
// This header declares macro `TURBO_DIE_IF_NULL`.

#pragma once

#include <stdint.h>

#include <utility>

#include <turbo/base/macros.h>

// TURBO_DIE_IF_NULL()
//
// `TURBO_DIE_IF_NULL` behaves as `KCHECK_NE` against `nullptr` but *also*
// "returns" its argument.  It is useful in initializers where statements (like
// `KCHECK_NE`) can't be used.  Outside initializers, prefer `KCHECK` or
// `KCHECK_NE`. `TURBO_DIE_IF_NULL` works for both raw pointers and (compatible)
// smart pointers including `std::unique_ptr` and `std::shared_ptr`; more
// generally, it works for any type that can be compared to nullptr_t.  For
// types that aren't raw pointers, `TURBO_DIE_IF_NULL` returns a reference to
// its argument, preserving the value category. Example:
//
//   Foo() : bar_(TURBO_DIE_IF_NULL(MethodReturningUniquePtr())) {}
//
// Use `KCHECK(ptr)` or `KCHECK(ptr != nullptr)` if the returned pointer is
// unused.
#define TURBO_DIE_IF_NULL(val) \
  ::turbo::log_internal::DieIfNull(__FILE__, __LINE__, #val, (val))

namespace turbo::log_internal {

    // Crashes the process after logging `exprtext` annotated at the `file` and
    // `line` location. Called when `TURBO_DIE_IF_NULL` fails. Calling this function
    // generates less code than its implementation would if inlined, for a slight
    // code size reduction each time `TURBO_DIE_IF_NULL` is called.
    TURBO_NORETURN TURBO_NOINLINE void DieBecauseNull(
            const char *file, int line, const char *exprtext);

    // Helper for `TURBO_DIE_IF_NULL`.
    template<typename T>
    TURBO_MUST_USE_RESULT T DieIfNull(const char *file, int line,
                                      const char *exprtext, T &&t) {
        if (TURBO_UNLIKELY(t == nullptr)) {
            // Call a non-inline helper function for a small code size improvement.
            DieBecauseNull(file, line, exprtext);
        }
        return std::forward<T>(t);
    }

}  // namespace turbo::log_internal
