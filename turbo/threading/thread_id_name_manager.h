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

#include <map>
#include <string>
#include <mutex>
#include <turbo/base/macros.h>
#include <turbo/threading/platform_thread.h>

template <typename T> struct DefaultSingletonTraits;

namespace turbo {

class TURBO_EXPORT ThreadIdNameManager {
 public:
  static ThreadIdNameManager* GetInstance();

  static const char* GetDefaultInternedString();

  // Register the mapping between a thread |id| and |handle|.
  void RegisterThread(PlatformThreadHandle::Handle handle, PlatformThreadId id);

  // Set the name for the given id.
  void SetName(PlatformThreadId id, const char* name);

  // Get the name for the given id.
  const char* GetName(PlatformThreadId id);

  // Remove the name for the given id.
  void RemoveName(PlatformThreadHandle::Handle handle, PlatformThreadId id);

 private:
  friend struct DefaultSingletonTraits<ThreadIdNameManager>;

  typedef std::map<PlatformThreadId, PlatformThreadHandle::Handle>
      ThreadIdToHandleMap;
  typedef std::map<PlatformThreadHandle::Handle, std::string*>
      ThreadHandleToInternedNameMap;
  typedef std::map<std::string, std::string*> NameToInternedNameMap;

  ThreadIdNameManager();
  ~ThreadIdNameManager();

  // lock_ protects the name_to_interned_name_, thread_id_to_handle_ and
  // thread_handle_to_interned_name_ maps.
  std::mutex lock_;

  NameToInternedNameMap name_to_interned_name_;
  ThreadIdToHandleMap thread_id_to_handle_;
  ThreadHandleToInternedNameMap thread_handle_to_interned_name_;

  // Treat the main process specially as there is no PlatformThreadHandle.
  std::string* main_process_name_;
  PlatformThreadId main_process_id_;

  TURBO_DISALLOW_COPY_AND_ASSIGN(ThreadIdNameManager);
};

}  // namespace turbo
