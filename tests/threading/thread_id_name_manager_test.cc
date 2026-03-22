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
#include <turbo/threading/thread_id_name_manager.h>

#include <turbo/threading/platform_thread.h>
#include <gtest/gtest.h>

typedef testing::Test ThreadIdNameManagerTest;

namespace {

TEST_F(ThreadIdNameManagerTest, ThreadNameInterning) {
  turbo::ThreadIdNameManager* manager = turbo::ThreadIdNameManager::GetInstance();

  turbo::PlatformThreadId a_id = turbo::PlatformThread::CurrentId();
  turbo::PlatformThread::SetName("First Name");
  std::string version = manager->GetName(a_id);

  turbo::PlatformThread::SetName("New name");
  EXPECT_NE(version, manager->GetName(a_id));
  turbo::PlatformThread::SetName("");
}

TEST_F(ThreadIdNameManagerTest, ResettingNameKeepsCorrectInternedValue) {
  turbo::ThreadIdNameManager* manager = turbo::ThreadIdNameManager::GetInstance();

  turbo::PlatformThreadId a_id = turbo::PlatformThread::CurrentId();
  turbo::PlatformThread::SetName("Test Name");
  std::string version = manager->GetName(a_id);

  turbo::PlatformThread::SetName("New name");
  EXPECT_NE(version, manager->GetName(a_id));

  turbo::PlatformThread::SetName("Test Name");
  EXPECT_EQ(version, manager->GetName(a_id));

  turbo::PlatformThread::SetName("");
}

}  // namespace
