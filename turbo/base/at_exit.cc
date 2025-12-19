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

#include <turbo/base/at_exit.h>
#include <cstddef>
#include <ostream>
#include <turbo/log/logging.h>

namespace turbo {

    // Keep a stack of registered AtExitManagers.  We always operate on the most
    // recent, and we should never have more than one outside of testing (for a
    // statically linked version of this library).  Testing may use the shadow
    // version of the constructor, and if we are building a dynamic library we may
    // end up with multiple AtExitManagers on the same process.  We don't protect
    // this for thread-safe access, since it will only be modified in testing.
    static AtExitManager *g_top_manager = nullptr;

    AtExitManager::AtExitManager() : next_manager_(g_top_manager) {
    // If multiple modules instantiate AtExitManagers they'll end up living in this
    // module... they have to coexist.
#if !defined(COMPONENT_BUILD)
        DKCHECK(!g_top_manager);
#endif
        g_top_manager = this;
    }

    AtExitManager::~AtExitManager() {
        if (!g_top_manager) {
            KLOG(FATAL) << "Tried to ~AtExitManager without an AtExitManager";
            return;
        }
        DKCHECK_EQ(this, g_top_manager);

        ProcessCallbacksNow();
        g_top_manager = next_manager_;
    }

    // static
    void AtExitManager::RegisterCallback(AtExitCallbackType func, void *param) {
        DKCHECK(func);
        if (!g_top_manager) {
            KLOG(FATAL) << "Tried to RegisterCallback without an AtExitManager";
            return;
        }

        std::unique_lock lock(g_top_manager->lock_);
        g_top_manager->stack_.push({func, param});
    }

    // static
    void AtExitManager::ProcessCallbacksNow() {
        if (!g_top_manager) {
            KLOG(FATAL) << "Tried to ProcessCallbacksNow without an AtExitManager";
            return;
        }

        std::unique_lock lock(g_top_manager->lock_);

        while (!g_top_manager->stack_.empty()) {
            Callback task = g_top_manager->stack_.top();
            task.func(task.param);
            g_top_manager->stack_.pop();
        }
    }

    AtExitManager::AtExitManager(bool shadow) : next_manager_(g_top_manager) {
        DKCHECK(shadow || !g_top_manager);
        g_top_manager = this;
    }

}  // namespace turbo
