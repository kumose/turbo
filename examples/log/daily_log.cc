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