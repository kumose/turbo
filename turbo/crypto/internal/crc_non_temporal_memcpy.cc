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

#include <cstddef>

#include <turbo/base/macros.h>
#include <turbo/crypto/crc32c.h>
#include <turbo/crypto/internal/crc_memcpy.h>
#include <turbo/crypto/internal/non_temporal_memcpy.h>
#include <turbo/strings/string_view.h>

namespace turbo::crc_internal {

    CRC32C CrcNonTemporalMemcpyEngine::Compute(void *TURBO_RESTRICT dst,
                                               const void *TURBO_RESTRICT src,
                                               std::size_t length,
                                               CRC32C initial_crc) const {
        constexpr size_t kBlockSize = 8192;
        CRC32C crc = initial_crc;

        const char *src_bytes = reinterpret_cast<const char *>(src);
        char *dst_bytes = reinterpret_cast<char *>(dst);

        // Copy + CRC loop - run 8k chunks until we are out of full chunks.
        std::size_t offset = 0;
        for (; offset + kBlockSize < length; offset += kBlockSize) {
            crc = turbo::extend_crc32c(crc,
                                       std::string_view(src_bytes + offset, kBlockSize));
            non_temporal_store_memcpy(dst_bytes + offset, src_bytes + offset,
                                      kBlockSize);
        }

        // Save some work if length is 0.
        if (offset < length) {
            std::size_t final_copy_size = length - offset;
            crc = extend_crc32c(crc,
                                std::string_view(src_bytes + offset, final_copy_size));

            non_temporal_store_memcpy(dst_bytes + offset, src_bytes + offset,
                                      final_copy_size);
        }

        return crc;
    }

    CRC32C CrcNonTemporalMemcpyAVXEngine::Compute(void *TURBO_RESTRICT dst,
                                                  const void *TURBO_RESTRICT src,
                                                  std::size_t length,
                                                  CRC32C initial_crc) const {
        constexpr size_t kBlockSize = 8192;
        CRC32C crc = initial_crc;

        const char *src_bytes = reinterpret_cast<const char *>(src);
        char *dst_bytes = reinterpret_cast<char *>(dst);

        // Copy + CRC loop - run 8k chunks until we are out of full chunks.
        std::size_t offset = 0;
        for (; offset + kBlockSize < length; offset += kBlockSize) {
            crc = extend_crc32c(crc, std::string_view(src_bytes + offset, kBlockSize));

            non_temporal_store_memcpy_avx(dst_bytes + offset, src_bytes + offset,
                                          kBlockSize);
        }

        // Save some work if length is 0.
        if (offset < length) {
            std::size_t final_copy_size = length - offset;
            crc = extend_crc32c(crc,
                                std::string_view(src_bytes + offset, final_copy_size));

            non_temporal_store_memcpy_avx(dst_bytes + offset, src_bytes + offset,
                                          final_copy_size);
        }

        return crc;
    }

}  // namespace turbo::crc_internal
