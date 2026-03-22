//
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

#include <turbo/log/logging.h>

void call0() {
    int x = 3, y = 5;
    KCHECK_EQ(2 * x, y) << "oops!";
}
void call1() {
    call0();
}
void call2() {
    call1();
}
void call3() {
    KLOG(INFO) << "hello world"<<"sd";
    call2();
}
void call4() {
    call3();
}
void call5() {
    call4();
    KLOG(INFO) << "hello world";
}
void call6() {
    KLOG(INFO) << "hello world";
    call5();
}
int main() {

    //turbo::initialize_log();
       turbo::setup_daily_file_sink("logs/daily_log.txt", 7, 60, false);
    auto now = turbo::Time::current_time();
    for(int i = 0; i < 100; i++) {
        KLOG(INFO).with_timestamp(now) << "hello world";
        now += turbo::Duration::hours(10);
        //KLOG(ERROR) << "error hello world";
    }
   // call6();
}