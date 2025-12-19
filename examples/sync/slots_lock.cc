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