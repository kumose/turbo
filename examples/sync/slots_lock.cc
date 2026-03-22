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
//
// Created by jeff on 24-5-31.
//

#include <turbo/synchronization/slots_locker.h>
#include <turbo/container/flat_hash_set.h>
#include <turbo/container/flat_hash_map.h>
#include <list>
int main() {
    std::vector<int> a;
    std::cout<<std::is_convertible<std::string, std::string_view>::value<<std::endl;
    std::cout<<std::is_convertible<std::string_view , std::string>::value<<std::endl;
    std::cout<<std::is_constructible<std::string_view , std::string>::value<<std::endl;
    std::cout<<std::forward<std::string_view>(std::string("abc"));
    std::cout<<turbo::is_container<turbo::flat_hash_map<std::string,std::string>>::value<<std::endl;
    std::cout<<turbo::is_container<std::vector<int>>::value<<std::endl;
    std::cout<<turbo::is_container<std::string>::value<<std::endl;
    std::cout<<turbo::is_container<int>::value<<std::endl;
    std::cout<<turbo::is_container_of<std::vector<int>, int>::value<<std::endl;
    std::cout<<turbo::is_container_of<std::vector<std::string>, std::string>::value<<std::endl;
    std::cout<<turbo::is_container_of<std::vector<std::string>, int>::value<<std::endl;
    std::cout<<turbo::is_container_of<std::list<std::string>, std::string>::value<<std::endl;
    std::cout<<turbo::is_container_of<std::set<std::string>, std::string>::value<<std::endl;
    std::cout<<turbo::is_container_of<turbo::flat_hash_set<std::string>, std::string>::value<<std::endl;
    turbo::SlotsLock<int, std::shared_mutex> ulok(10);
    {
        turbo::UniqueLockGuard<turbo::SlotsLock<int, std::shared_mutex>> gard(&ulok, 10);
    }

    {
        turbo::SharedLockGuard<turbo::SlotsLock<int, std::shared_mutex>> gard(&ulok, 10);
    }

    {
        turbo::SharedLockGuard<turbo::SlotsLock<int, std::shared_mutex>> sgard(&ulok, 10);
    }

    {
        turbo::UniqueLockGuard<turbo::SlotsLock<int, std::shared_mutex>> gard(&ulok, std::vector<int>{10, 20});
    }

    {
        turbo::SharedLockGuard<turbo::SlotsLock<int, std::shared_mutex>> gard(&ulok, std::vector<int>{10, 20});
    }


    {
        turbo::SharedLockGuard<turbo::SlotsLock<int, std::shared_mutex>> sgard(&ulok, std::initializer_list<int>{10, 20});
    }

    turbo::SlotsLock<std::string, std::shared_mutex> strings_lok(10);
    {
        turbo::UniqueLockGuard<turbo::SlotsLock<std::string, std::shared_mutex>> gard(&strings_lok, "10");
    }

    {
        std::string a("10");
        turbo::UniqueLockGuard<turbo::SlotsLock<std::string, std::shared_mutex>> gard(&strings_lok, a);
    }

    {
        turbo::SharedLockGuard<turbo::SlotsLock<std::string, std::shared_mutex>> gard(&strings_lok, std::string_view ("10"));
    }

    {
        std::vector<std::string_view> aa{"10", "20"};
        turbo::UniqueLockGuard<turbo::SlotsLock<std::string, std::shared_mutex>> gard(&strings_lok, aa);
    }

    {
        std::vector<std::string_view> aa{"10", "20"};
        turbo::SharedLockGuard<turbo::SlotsLock<std::string, std::shared_mutex>> gard(&strings_lok, aa);
    }

    {
        std::vector<std::string> bb{"10", "20"};
        turbo::UniqueLockGuard<turbo::SlotsLock<std::string, std::shared_mutex>> gard(&strings_lok, bb);
    }

    {
        std::vector<std::string> bb{"10", "20"};
        const auto &cc = bb;
        turbo::SharedLockGuard<turbo::SlotsLock<std::string, std::shared_mutex>> gard(&strings_lok, cc);
    }

    turbo::SlotsLock<std::string_view, std::shared_mutex> string_view_lok(10);
    {
        turbo::UniqueLockGuard<turbo::SlotsLock<std::string_view, std::shared_mutex>> gard(&string_view_lok, "10");
    }

    {
        std::vector<std::string> aa{"10", "20"};
        turbo::UniqueLockGuard<turbo::SlotsLock<std::string_view, std::shared_mutex>> gard(&string_view_lok, aa);
    }
    {
        std::vector<std::string> aa{"10", "20"};
        turbo::SharedLockGuard<turbo::SlotsLock<std::string_view, std::shared_mutex>> gard(&string_view_lok, aa);
    }

    {
        std::set<std::string> aa{"10", "20"};
        turbo::SharedLockGuard<turbo::SlotsLock<std::string_view, std::shared_mutex>> gard(&string_view_lok, aa);
    }

    {
        std::list<std::string> aa{"10", "20"};
        turbo::SharedLockGuard<turbo::SlotsLock<std::string_view, std::shared_mutex>> gard(&string_view_lok, aa);
    }

    {
        std::vector<std::string_view> bb{"10", "20"};
        turbo::UniqueLockGuard<turbo::SlotsLock<std::string_view, std::shared_mutex>> gard(&string_view_lok, bb);
    }

    {
        std::vector<std::string_view> bb{"10", "20"};
        turbo::SharedLockGuard<turbo::SlotsLock<std::string_view, std::shared_mutex>> gard(&string_view_lok, bb);
    }
    /*
     * * bad case
    {
        std::vector<int> bb{11, 12};
        turbo::UniqueLockGuard<turbo::SlotsLock<std::string_view, std::shared_mutex>> gard(&string_view_lok, bb);
    }*/

}