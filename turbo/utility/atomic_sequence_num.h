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
