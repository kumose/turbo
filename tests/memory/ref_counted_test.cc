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
