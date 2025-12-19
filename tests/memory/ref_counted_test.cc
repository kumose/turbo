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


#include <turbo/memory/ref_counted.h>
#include <gtest/gtest.h>

namespace {

class SelfAssign : public turbo::RefCounted<SelfAssign> {
  friend class turbo::RefCounted<SelfAssign>;

  ~SelfAssign() {}
};

class CheckDerivedMemberAccess : public scoped_refptr<SelfAssign> {
 public:
  CheckDerivedMemberAccess() {
    // This shouldn't compile if we don't have access to the member variable.
    SelfAssign** pptr = &ptr_;
    EXPECT_EQ(*pptr, ptr_);
  }
};

class ScopedRefPtrToSelf : public turbo::RefCounted<ScopedRefPtrToSelf> {
 public:
  ScopedRefPtrToSelf() : self_ptr_(this) {}

  static bool was_destroyed() { return was_destroyed_; }

  void SelfDestruct() { self_ptr_ = nullptr; }

 private:
  friend class turbo::RefCounted<ScopedRefPtrToSelf>;
  ~ScopedRefPtrToSelf() { was_destroyed_ = true; }

  static bool was_destroyed_;

  scoped_refptr<ScopedRefPtrToSelf> self_ptr_;
};

bool ScopedRefPtrToSelf::was_destroyed_ = false;

}  // end namespace

TEST(RefCountedUnitTest, TestSelfAssignment) {
  SelfAssign* p = new SelfAssign;
  scoped_refptr<SelfAssign> var(p);
  var = var;
  EXPECT_EQ(var.get(), p);
}

TEST(RefCountedUnitTest, ScopedRefPtrMemberAccess) {
  CheckDerivedMemberAccess check;
}

TEST(RefCountedUnitTest, ScopedRefPtrToSelf) {
  ScopedRefPtrToSelf* check = new ScopedRefPtrToSelf();
  EXPECT_FALSE(ScopedRefPtrToSelf::was_destroyed());
  check->SelfDestruct();
  EXPECT_TRUE(ScopedRefPtrToSelf::was_destroyed());
}

TEST(RefCountedUnitTest, ScopedRefPtrBooleanOperations) {
  scoped_refptr<SelfAssign> p1 = new SelfAssign;
  scoped_refptr<SelfAssign> p2;

  EXPECT_TRUE(p1);
  EXPECT_FALSE(!p1);

  EXPECT_TRUE(!p2);
  EXPECT_FALSE(p2);

  EXPECT_NE(p1, p2);

  SelfAssign* raw_p = new SelfAssign;
  p2 = raw_p;
  EXPECT_NE(p1, p2);
  EXPECT_EQ(raw_p, p2);

  p2 = p1;
  EXPECT_NE(raw_p, p2);
  EXPECT_EQ(p1, p2);
}

TEST(RefCountedUnitTest, ScopedRefPtrMoveCtor)
{
  scoped_refptr<SelfAssign> p1 = new SelfAssign;
  EXPECT_TRUE(p1);

  scoped_refptr<SelfAssign> p2(std::move(p1));
  EXPECT_TRUE(p2);
  EXPECT_FALSE(p1);

  scoped_refptr<SelfAssign> p3(std::move(p1));
  EXPECT_FALSE(p3);
}
