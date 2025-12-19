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
