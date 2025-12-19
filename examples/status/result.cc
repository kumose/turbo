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
//
// Created by jeff on 24-5-31.
//

#include <turbo/utility/status.h>
#include <turbo/log/logging.h>
#include <turbo/debugging/stacktrace.h>

turbo::Result<int> result_not_ok() {
    return turbo::internal_error("not ok");
}

turbo::Result<int> result_ok() {
    return 1;
}


turbo::Status call_ok() {
    TURBO_MOVE_OR_RAISE(auto r, result_ok());
    std::cout << "this should be 1: " << r << std::endl;
    return turbo::OkStatus();
}

turbo::Status call_not_ok() {
    TURBO_MOVE_OR_RAISE(auto r, result_not_ok(), "result_not_ok");
    (void)r;
    std::cout << "this should not be printed call_not_ok" << std::endl;
    return turbo::OkStatus();
}

turbo::Status result_not_ok1() {
    TURBO_RETURN_NOT_OK(call_not_ok());
    std::cout << "this should not be printed result_not_ok1" << std::endl;
    return turbo::OkStatus();
}

void status_not_ok_warn() {
    TURBO_WARN_NOT_OK(call_not_ok());
    TURBO_WARN_NOT_OK(call_not_ok(),"");
    TURBO_WARN_NOT_OK(call_not_ok(),"abc-", 2);
    std::cout << "this should be printed status_not_ok_warn" << std::endl;
}

void status_not_ok_abort() {
    TURBO_ABORT_NOT_OK(call_not_ok());
    TURBO_ABORT_NOT_OK(call_not_ok(),"");
    TURBO_ABORT_NOT_OK(call_not_ok(),"%s-%d", "abc", 2);
    std::cout << "this should not be printed status_not_ok_abort" << std::endl;
}

turbo::Status status_else() {
    TURBO_RETURN_NOT_OK_ELSE(call_not_ok(),[](){
        std::cout<<"status_else"<<std::endl;
    }());
    return turbo::OkStatus();
}

turbo::Status call_not_ok1() {
    auto r = call_not_ok();
    std::cout<<r<<std::endl;
    TURBO_RETURN_NOT_OK(r, "stream message call_not_ok1");
    std::cout << "this should not be printed call_not_ok1" << std::endl;
    return turbo::OkStatus();
}

turbo::Status b_stack_0() {
    return turbo::invalid_argument_error("").push_stack(TURBO_STATUS_TRACE_PARAM,"b_stack_0");
}

turbo::Status b_stack_1() {
    auto rs = b_stack_0();
    rs.push_stack(TURBO_STATUS_TRACE_PARAM,"b_stack_1");
    return rs;
}

turbo::Status b_stack_2() {
    auto rs = b_stack_0();
    rs.push_stack(TURBO_STATUS_TRACE_PARAM,"b_stack_2");
    return rs;
}

void a0() {
    void* result[10];
    int depth = turbo::GetStackTrace(result, 10, 1);
    std::cout<<depth<<std::endl;
    for(int i = 0; i < depth; i++) {
        std::cout<<std::string_view(static_cast<const char*>(result[i]))<<std::endl;
    }
    KLOG(FATAL)<<0;
}

void a1() {
   a0();
}
void a2() {
    a1();
}

void a3() {
    a2();
}
/*
void result_call_not_abort() {
    RESULT_ASSIGN_OR_ABORT(auto r, result_not_ok());
    RESULT_ASSIGN_OR_ABORT(auto r1, result_not_ok(),"result_not_ok");
    RESULT_ASSIGN_OR_ABORT(auto r2, result_not_ok(),"abc-", 2);
    (void)r;
}
*/
turbo::Status call_not_ok2() {
    TURBO_RETURN_NOT_OK(call_not_ok1(), " stream message call_not_ok2");
    std::cout << "this should not be printed call_not_ok2" << std::endl;
    return turbo::OkStatus();
}

enum TTT {
    T_1 = 300
};

enum T_u8 : uint8_t {
    T_u8 = 100
};

enum T_large : uint64_t {
    T_l = 300
};

enum class T_large_c : uint64_t {
    T_l_c = 300
};

int main() {
    auto r = call_ok();
    std::cout << "this should be ok: " << r << std::endl;
    r = call_not_ok();
    std::cout << "this should be printed: " << r << std::endl;
    std::cout << "---------------------------------------" <<std::endl;
    auto r1 = call_not_ok1();
    std::cout << "call_not_ok1: " << r1 << std::endl;
    r = call_not_ok2();
    std::cout << "this should be printed: " << r << std::endl;
    r = result_not_ok1();
    std::cout << "this should be printed: " << r << std::endl;
    status_not_ok_warn();
    //result_call_not_abort();
    //result_call_not_abort();
    int n1;
    std::cout << result_ok().try_value(&n1)<<std::endl;

    float n2;
    std::cout << result_ok().try_value(&n2)<<std::endl;

    std::cout <<b_stack_2()<<std::endl;
    auto s = turbo::make_status(300, "anv");
    std::cout <<s.is_raw_code(3)<<std::endl;

    std::cout <<s.is_raw_code(TTT::T_1)<<std::endl;

    /// static_assert fail
    /// when compare, we always convert to int to compare
    //std::cout <<s.is_raw_code(T_large::T_l)<<std::endl;
    //std::cout <<s.is_raw_code(T_large_c::T_l_c)<<std::endl;

    std::cout <<s.is_raw_code(T_u8::T_u8)<<std::endl;

    /// type error
    /*
    std::string n3;
    std::cout << result_ok().try_value(&n3)<<std::endl;
     */
    //status_else();
    //a3();
    return 0;
}