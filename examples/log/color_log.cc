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