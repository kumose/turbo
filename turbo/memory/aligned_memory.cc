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
#include <turbo/memory/aligned_memory.h>
#include <turbo/log/logging.h>

#if defined(OS_ANDROID)
#include <malloc.h>
#endif

namespace turbo {

    void *AlignedAlloc(size_t size, size_t alignment) {
        DKCHECK_GT(size, 0U);
        DKCHECK_EQ(alignment & (alignment - 1), 0U);
        DKCHECK_EQ(alignment % sizeof(void *), 0U);
        void *ptr = nullptr;
#if defined(COMPILER_MSVC)
        ptr = _aligned_malloc(size, alignment);
      // Android technically supports posix_memalign(), but does not expose it in
      // the current version of the library headers used by Chrome.  Luckily,
      // memalign() on Android returns pointers which can safely be used with
      // free(), so we can use it instead.  Issue filed to document this:
      // http://code.google.com/p/android/issues/detail?id=35391
#elif defined(OS_ANDROID)
        ptr = memalign(alignment, size);
#else
        if (posix_memalign(&ptr, alignment, size))
            ptr = nullptr;
#endif
        // Since aligned allocations may fail for non-memory related reasons, force a
        // crash if we encounter a failed allocation; maintaining consistent behavior
        // with a normal allocation failure in Chrome.
        if (!ptr) {
            DKLOG(ERROR) << "If you crashed here, your aligned allocation is incorrect: "
                        << "size=" << size << ", alignment=" << alignment;
            KCHECK(false);
        }
        // Sanity check alignment just to be safe.
        DKCHECK_EQ(reinterpret_cast<uintptr_t>(ptr) & (alignment - 1), 0U);
        return ptr;
    }

}  // namespace turbo
