// Copyright (C) Kumo inc. and its affiliates.
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
#include <iostream>
#include <turbo/log/log_filename.h>
#include <turbo/log/logging.h>
#include <glog/logging.h>
/// PROXY_GLOG_TEST for testing add "from glog"
/// at start of message, no need for prd environment
#define PROXY_GLOG_TEST
#include "turbo/log/glog.h"



int main(int argc, char **argv) {

    turbo::BridgeFromGoogleLogging bridge;

    uint64_t i = 0;
    while (i < 1000) {
        LOG(INFO)<<i<<" glog_proxy test";
        i++;
    }
    return 0;
}
