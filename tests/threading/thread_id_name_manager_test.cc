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
