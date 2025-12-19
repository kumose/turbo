// Copyright (C) 2024 Kumo group inc.
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

#include <any>
#include <functional>
#include <memory>
#include <utility>
#include <turbo/base/macros.h>

namespace turbo {

    struct TURBO_EXPORT AtForkHandler {
        using CallbackBefore = std::function<std::any()>;
        using CallbackAfter = std::function<void(std::any)>;

        // The before-fork callback can return an arbitrary token (wrapped in std::any)
        // that will passed as-is to after-fork callbacks.  This can ensure that any
        // resource necessary for after-fork handling is kept alive.
        CallbackBefore before;
        CallbackAfter parent_after;
        CallbackAfter child_after;

        AtForkHandler() = default;

        explicit AtForkHandler(CallbackAfter child_after)
                : child_after(std::move(child_after)) {}

        AtForkHandler(CallbackBefore before, CallbackAfter parent_after,
                      CallbackAfter child_after)
                : before(std::move(before)),
                  parent_after(std::move(parent_after)),
                  child_after(std::move(child_after)) {}
    };

    // Register the given at-fork handlers. Their intended lifetime should be tracked by
    // calling code using an owning shared_ptr.
    TURBO_EXPORT void register_at_fork(std::weak_ptr<AtForkHandler>);

}  // namespace turbo
