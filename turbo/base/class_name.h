// Copyright (C) 2024 Kumo inc.
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
#pragma once

#include <typeinfo>
#include <string>                                // std::string

namespace turbo {

    std::string demangle(const char *name);

    namespace {
        template<typename T>
        struct ClassNameHelper {
            static std::string name;
        };
        template<typename T> std::string ClassNameHelper<T>::name = demangle(typeid(T).name());
    }

    // Get name of class |T|, in std::string.
    template<typename T>
    const std::string &class_name_str() {
        // We don't use static-variable-inside-function because before C++11
        // local static variable is not guaranteed to be thread-safe.
        return ClassNameHelper<T>::name;
    }

    // Get name of class |T|, in const char*.
    // Address of returned name never changes.
    template<typename T>
    const char *class_name() {
        return class_name_str<T>().c_str();
    }

    // Get typename of |obj|, in std::string
    template<typename T>
    std::string class_name_str(T const &obj) {
        return demangle(typeid(obj).name());
    }

}  // namespace turbo
