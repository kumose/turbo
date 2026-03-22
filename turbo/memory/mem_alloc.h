//
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
#pragma once


#include <turbo/base/macros.h>
#include <turbo/base/internal/raw_logging.h>
#include <cstdlib>

namespace turbo {

    TURBO_ATTRIBUTE_RETURNS_NONNULL inline void *safe_malloc(size_t Sz) {
        void *Result = std::malloc(Sz);
        if (Result == nullptr) {
            // It is implementation-defined whether allocation occurs if the space
            // requested is zero (ISO/IEC 9899:2018 7.22.3). Retry, requesting
            // non-zero, if the space requested was zero.
            if (Sz == 0)
                return safe_malloc(1);
            TURBO_RAW_LOG(FATAL, "Allocation failed");
        }
        return Result;
    }

    TURBO_ATTRIBUTE_RETURNS_NONNULL inline void *safe_calloc(size_t Count,
                                                            size_t Sz) {
        void *Result = std::calloc(Count, Sz);
        if (Result == nullptr) {
            // It is implementation-defined whether allocation occurs if the space
            // requested is zero (ISO/IEC 9899:2018 7.22.3). Retry, requesting
            // non-zero, if the space requested was zero.
            if (Count == 0 || Sz == 0)
                return safe_malloc(1);
            TURBO_RAW_LOG(FATAL, "Allocation failed");
        }
        return Result;
    }

    TURBO_ATTRIBUTE_RETURNS_NONNULL inline void *safe_realloc(void *Ptr, size_t Sz) {
        void *Result = std::realloc(Ptr, Sz);
        if (Result == nullptr) {
            // It is implementation-defined whether allocation occurs if the space
            // requested is zero (ISO/IEC 9899:2018 7.22.3). Retry, requesting
            // non-zero, if the space requested was zero.
            if (Sz == 0)
                return safe_malloc(1);
            TURBO_RAW_LOG(FATAL, "Allocation failed");
        }
        return Result;
    }

    /// Allocate a buffer of memory with the given size and alignment.
    ///
    /// When the compiler supports aligned operator new, this will use it to to
    /// handle even over-aligned allocations.
    ///
    /// However, this doesn't make any attempt to leverage the fancier techniques
    /// like posix_memalign due to portability. It is mostly intended to allow
    /// compatibility with platforms that, after aligned allocation was added, use
    /// reduced default alignment.
    TURBO_ATTRIBUTE_RETURNS_NONNULL TURBO_ATTRIBUTE_RETURNS_NOALIAS void *
    allocate_buffer(size_t Size, size_t Alignment);

    /// Deallocate a buffer of memory with the given size and alignment.
    ///
    /// If supported, this will used the sized delete operator. Also if supported,
    /// this will pass the alignment to the delete operator.
    ///
    /// The pointer must have been allocated with the corresponding new operator,
    /// most likely using the above helper.
    void deallocate_buffer(void *Ptr, size_t Size, size_t Alignment);

} // namespace llvm