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

#include <gtest/gtest.h>
#include <turbo/utility/errno.h>

class ErrnoTest : public ::testing::Test{
protected:
    ErrnoTest(){
    };
    virtual ~ErrnoTest(){};
    virtual void SetUp() {
    };
    virtual void TearDown() {
    };
};

#define ESTOP -114
#define EBREAK -115
#define ESTH -116
#define EOK -117
#define EMYERROR -30

TURBO_REGISTER_ERRNO(ESTOP, "the thread is stopping")
TURBO_REGISTER_ERRNO(EBREAK, "the thread is interrupted")
TURBO_REGISTER_ERRNO(ESTH, "something happened")
TURBO_REGISTER_ERRNO(EOK, "OK!")
TURBO_REGISTER_ERRNO(EMYERROR, "my error");

TEST_F(ErrnoTest, system_errno) {
    errno = EPIPE;
    ASSERT_STREQ("Broken pipe", km_error());
    ASSERT_STREQ("Interrupted system call", km_error(EINTR));
}

TEST_F(ErrnoTest, customized_errno) {
    ASSERT_STREQ("the thread is stopping", km_error(ESTOP));
    ASSERT_STREQ("the thread is interrupted", km_error(EBREAK));
    ASSERT_STREQ("something happened", km_error(ESTH));
    ASSERT_STREQ("OK!", km_error(EOK));
    ASSERT_STREQ("Unknown error 1000", km_error(1000));
    
    errno = ESTOP;
    printf("Something got wrong, %m\n");
    printf("Something got wrong, %s\n", km_error());
}
