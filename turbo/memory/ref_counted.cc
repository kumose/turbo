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
#include <turbo/memory/ref_counted.h>

namespace turbo {

namespace subtle {

bool RefCountedThreadSafeBase::HasOneRef() const {
  return const_cast<RefCountedThreadSafeBase*>(this)->ref_count_.load() == 1;
}

RefCountedThreadSafeBase::RefCountedThreadSafeBase() : ref_count_(0) {
#ifndef NDEBUG
  in_dtor_ = false;
#endif
}

RefCountedThreadSafeBase::~RefCountedThreadSafeBase() {
#ifndef NDEBUG
  DKCHECK(in_dtor_) << "RefCountedThreadSafe object deleted without "
                      "calling Release()";
#endif
}

void RefCountedThreadSafeBase::AddRef() const {
#ifndef NDEBUG
  DKCHECK(!in_dtor_);
#endif
    ref_count_.fetch_add(1);
}

bool RefCountedThreadSafeBase::Release() const {
#ifndef NDEBUG
  DKCHECK(!in_dtor_);
  DKCHECK(!ref_count_.load());
#endif
  if (ref_count_.fetch_sub(1) != 1) {
#ifndef NDEBUG
    in_dtor_ = true;
#endif
    return true;
  }
  return false;
}

}  // namespace subtle

}  // namespace turbo
