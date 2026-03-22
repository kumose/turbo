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

#include <atomic>
#include <turbo/base/macros.h>

namespace turbo {

    class AtomicSequenceNumber;

    // Static (POD) AtomicSequenceNumber that MUST be used in global scope (or
    // non-function scope) ONLY. This implementation does not generate any static
    // initializer.  Note that it does not implement any constructor which means
    // that its fields are not initialized except when it is stored in the global
    // data section (.data in ELF). If you want to allocate an atomic sequence
    // number on the stack (or heap), please use the AtomicSequenceNumber class
    // declared below.
    class StaticAtomicSequenceNumber {
    public:
        inline int GetNext() {
            return seq_.fetch_add(1);
        }

    private:
        friend class AtomicSequenceNumber;

        inline void Reset() {
            seq_.store(0);
        }

        std::atomic<int32_t> seq_;
    };

    // AtomicSequenceNumber that can be stored and used safely (i.e. its fields are
    // always initialized as opposed to StaticAtomicSequenceNumber declared above).
    // Please use StaticAtomicSequenceNumber if you want to declare an atomic
    // sequence number in the global scope.
    class AtomicSequenceNumber {
    public:
        AtomicSequenceNumber() {
            seq_.Reset();
        }

        inline int GetNext() {
            return seq_.GetNext();
        }

    private:
        StaticAtomicSequenceNumber seq_;
        TURBO_DISALLOW_COPY_AND_ASSIGN(AtomicSequenceNumber);
    };

}  // namespace turbo
