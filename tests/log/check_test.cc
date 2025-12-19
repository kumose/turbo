//
// Copyright (C) 2024 EA group inc.
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

#include <turbo/log/check.h>

#define TURBO_TEST_CHECK KCHECK
#define TURBO_TEST_CHECK_OK KCHECK_OK
#define TURBO_TEST_CHECK_EQ KCHECK_EQ
#define TURBO_TEST_CHECK_NE KCHECK_NE
#define TURBO_TEST_CHECK_GE KCHECK_GE
#define TURBO_TEST_CHECK_LE KCHECK_LE
#define TURBO_TEST_CHECK_GT KCHECK_GT
#define TURBO_TEST_CHECK_LT KCHECK_LT
#define TURBO_TEST_CHECK_STREQ KCHECK_STREQ
#define TURBO_TEST_CHECK_STRNE KCHECK_STRNE
#define TURBO_TEST_CHECK_STRCASEEQ KCHECK_STRCASEEQ
#define TURBO_TEST_CHECK_STRCASENE KCHECK_STRCASENE

#define TURBO_TEST_DCHECK DKCHECK
#define TURBO_TEST_DCHECK_OK DKCHECK_OK
#define TURBO_TEST_DCHECK_EQ DKCHECK_EQ
#define TURBO_TEST_DCHECK_NE DKCHECK_NE
#define TURBO_TEST_DCHECK_GE DKCHECK_GE
#define TURBO_TEST_DCHECK_LE DKCHECK_LE
#define TURBO_TEST_DCHECK_GT DKCHECK_GT
#define TURBO_TEST_DCHECK_LT DKCHECK_LT
#define TURBO_TEST_DCHECK_STREQ DKCHECK_STREQ
#define TURBO_TEST_DCHECK_STRNE DKCHECK_STRNE
#define TURBO_TEST_DCHECK_STRCASEEQ DKCHECK_STRCASEEQ
#define TURBO_TEST_DCHECK_STRCASENE DKCHECK_STRCASENE

#define TURBO_TEST_QCHECK QKCHECK
#define TURBO_TEST_QCHECK_OK QKCHECK_OK
#define TURBO_TEST_QCHECK_EQ QKCHECK_EQ
#define TURBO_TEST_QCHECK_NE QKCHECK_NE
#define TURBO_TEST_QCHECK_GE QKCHECK_GE
#define TURBO_TEST_QCHECK_LE QKCHECK_LE
#define TURBO_TEST_QCHECK_GT QKCHECK_GT
#define TURBO_TEST_QCHECK_LT QKCHECK_LT
#define TURBO_TEST_QCHECK_STREQ QKCHECK_STREQ
#define TURBO_TEST_QCHECK_STRNE QKCHECK_STRNE
#define TURBO_TEST_QCHECK_STRCASEEQ QKCHECK_STRCASEEQ
#define TURBO_TEST_QCHECK_STRCASENE QKCHECK_STRCASENE

#include <gtest/gtest.h>
#include <tests/log/check_test_impl.inc>
