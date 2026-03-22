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
// Created by jeff on 24-6-4.
//

#include <turbo/memory/mem_alloc.h>
#include <new>

namespace turbo {
// These are out of line to have __cpp_aligned_new not affect ABI.
    TURBO_ATTRIBUTE_RETURNS_NONNULL TURBO_ATTRIBUTE_RETURNS_NOALIAS void *
    allocate_buffer(size_t Size, size_t Alignment) {
        return ::operator new(Size
#ifdef __cpp_aligned_new
                ,
                              std::align_val_t(Alignment)
#endif
        );
    }

    void deallocate_buffer(void *Ptr, size_t Size, size_t Alignment) {
        ::operator delete(Ptr
#ifdef __cpp_sized_deallocation
                ,
                          Size
#endif
#ifdef __cpp_aligned_new
                ,
                          std::align_val_t(Alignment)
#endif
        );
    }
}  // namespace turbo