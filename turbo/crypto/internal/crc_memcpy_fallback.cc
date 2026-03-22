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

#include <cstring>
#include <memory>

#include <turbo/base/macros.h>
#include <turbo/crypto/crc32c.h>
#include <turbo/crypto/internal/crc_memcpy.h>
#include <turbo/strings/string_view.h>

namespace turbo::crc_internal {

    turbo::CRC32C FallbackCrcMemcpyEngine::Compute(void *TURBO_RESTRICT dst,
                                                     const void *TURBO_RESTRICT src,
                                                     std::size_t length,
                                                   CRC32C initial_crc) const {
        constexpr size_t kBlockSize = 8192;
        turbo::CRC32C crc = initial_crc;

        const char *src_bytes = reinterpret_cast<const char *>(src);
        char *dst_bytes = reinterpret_cast<char *>(dst);

        // Copy + CRC loop - run 8k chunks until we are out of full chunks.  CRC
        // then copy was found to be slightly more efficient in our test cases.
        std::size_t offset = 0;
        for (; offset + kBlockSize < length; offset += kBlockSize) {
            crc = turbo::extend_crc32c(crc,
                                       std::string_view(src_bytes + offset, kBlockSize));
            memcpy(dst_bytes + offset, src_bytes + offset, kBlockSize);
        }

        // Save some work if length is 0.
        if (offset < length) {
            std::size_t final_copy_size = length - offset;
            crc = turbo::extend_crc32c(
                    crc, std::string_view(src_bytes + offset, final_copy_size));
            memcpy(dst_bytes + offset, src_bytes + offset, final_copy_size);
        }

        return crc;
    }

// Compile the following only if we don't have
#if !defined(TURBO_INTERNAL_HAVE_X86_64_ACCELERATED_CRC_MEMCPY_ENGINE) && \
    !defined(TURBO_INTERNAL_HAVE_ARM_ACCELERATED_CRC_MEMCPY_ENGINE)

    CrcMemcpy::ArchSpecificEngines CrcMemcpy::GetArchSpecificEngines() {
        CrcMemcpy::ArchSpecificEngines engines;
        engines.temporal = new FallbackCrcMemcpyEngine();
        engines.non_temporal = new FallbackCrcMemcpyEngine();
        return engines;
    }

    std::unique_ptr<CrcMemcpyEngine> CrcMemcpy::GetTestEngine(int /*vector*/,
                                                              int /*integer*/) {
        return std::make_unique<FallbackCrcMemcpyEngine>();
    }

#endif  // !TURBO_INTERNAL_HAVE_X86_64_ACCELERATED_CRC_MEMCPY_ENGINE &&
    // !TURBO_INTERNAL_HAVE_ARM_ACCELERATED_CRC_MEMCPY_ENGINE
}  // namespace turbo::crc_internal
