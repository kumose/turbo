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


#include <errno.h>

#include <turbo/base/scoped_clear_errno.h>
#include <gtest/gtest.h>

namespace turbo {

TEST(ScopedClearErrno, TestNoError) {
  errno = 1;
  {
    ScopedClearErrno clear_error;
    EXPECT_EQ(0, errno);
  }
  EXPECT_EQ(1, errno);
}

TEST(ScopedClearErrno, TestError) {
  errno = 1;
  {
    ScopedClearErrno clear_error;
    errno = 2;
  }
  EXPECT_EQ(2, errno);
}

}  // namespace turbo
