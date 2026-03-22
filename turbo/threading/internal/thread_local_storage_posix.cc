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
#include <turbo/threading/internal/thread_local_storage.h>

#include <turbo/log/logging.h>

namespace turbo {

namespace internal {

bool PlatformThreadLocalStorage::AllocTLS(TLSKey* key) {
  return !pthread_key_create(key,
      turbo::internal::PlatformThreadLocalStorage::OnThreadExit);
}

void PlatformThreadLocalStorage::FreeTLS(TLSKey key) {
  int ret = pthread_key_delete(key);
  DKCHECK_EQ(ret, 0);
}

void* PlatformThreadLocalStorage::GetTLSValue(TLSKey key) {
  return pthread_getspecific(key);
}

void PlatformThreadLocalStorage::SetTLSValue(TLSKey key, void* value) {
  int ret = pthread_setspecific(key, value);
  DKCHECK_EQ(ret, 0);
}

}  // namespace internal

}  // namespace turbo
