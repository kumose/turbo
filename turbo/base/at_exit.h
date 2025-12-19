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

#include <stack>
#include <mutex>
#include <turbo/base/macros.h>

namespace turbo {

    // This class provides a facility similar to the CRT atexit(), except that
    // we control when the callbacks are executed. Under Windows for a DLL they
    // happen at a really bad time and under the loader lock. This facility is
    // mostly used by turbo::Singleton.
    //
    // The usage is simple. Early in the main() or WinMain() scope create an
    // AtExitManager object on the stack:
    // int main(...) {
    //    turbo::AtExitManager exit_manager;
    //
    // }
    // When the exit_manager object goes out of scope, all the registered
    // callbacks and singleton destructors will be called.

    class TURBO_EXPORT AtExitManager {
    public:
        typedef void (*AtExitCallbackType)(void *);

        AtExitManager();

        // The dtor calls all the registered callbacks. Do not try to register more
        // callbacks after this point.
        ~AtExitManager();

        // Registers the specified function to be called at exit. The prototype of
        // the callback function is void func(void*).
        static void RegisterCallback(AtExitCallbackType func, void *param);

        // Calls the functions registered with RegisterCallback in LIFO order. It
        // is possible to register new callbacks after calling this function.
        static void ProcessCallbacksNow();

    protected:
        // This constructor will allow this instance of AtExitManager to be created
        // even if one already exists.  This should only be used for testing!
        // AtExitManagers are kept on a global stack, and it will be removed during
        // destruction.  This allows you to shadow another AtExitManager.
        explicit AtExitManager(bool shadow);

    private:
        struct Callback {
            AtExitCallbackType func;
            void *param;
        };
        std::mutex lock_;
        std::stack<Callback> stack_;
        AtExitManager *next_manager_;  // Stack of managers to allow shadowing.

        TURBO_DISALLOW_COPY_AND_ASSIGN(AtExitManager);
    };

}  // namespace turbo
