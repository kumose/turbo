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
