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
