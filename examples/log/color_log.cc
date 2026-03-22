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
    KLOG(INFO) << "hello world call3"<<"sd";
    call2();
}
void call4() {
    call3();
}
void call5() {
    call4();
    KLOG(INFO) << "hello world call5";
}
void call6() {
    KLOG(INFO) << "hello world call6";
    call5();
}
int main() {

    //turbo::initialize_log();
       turbo::setup_color_stderr_sink();
    for(int i = 0; i < 100; i++) {
        KLOG(INFO) << "hello world "<<i;
        KLOG(WARNING) << "hello world";
        KLOG(ERROR) << "hello world";
    }
    turbo::set_min_log_level(turbo::LogSeverityAtLeast::kWarning);
    turbo::set_global_vlog_level(20);
    VKLOG(1) << "hello world 1";
    VKLOG(2) << "hello world 2";
    VKLOG(3) << "hello world 3";
    VKLOG(20) << "hello world 20";
    VKLOG(21) << "hello world 21";
    call6();
}