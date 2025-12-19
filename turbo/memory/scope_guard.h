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

#include <turbo/meta/type_traits.h>
#include <turbo/base/macros.h>
#include <utility>

namespace turbo {

template<typename Callback,
         typename = std::enable_if<is_result_void<Callback>::value>>
class ScopeGuard;

template<typename Callback>
ScopeGuard<Callback> MakeScopeGuard(Callback&& callback) noexcept;

// ScopeGuard is a simple implementation to guarantee that
// a function is executed upon leaving the current scope.
template<typename Callback>
class ScopeGuard<Callback> {
public:
    ScopeGuard(ScopeGuard&& other) noexcept
        : _callback(std::move(other._callback))
        , _dismiss(other._dismiss) {
        other.dismiss();
    }

    ~ScopeGuard() noexcept {
        if(!_dismiss) {
            _callback();
        }
    }

    void dismiss() noexcept {
        _dismiss = true;
    }

    ScopeGuard() = delete;
    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;
    ScopeGuard& operator=(ScopeGuard&&) = delete;

private:
// Only MakeScopeGuard and move constructor can create ScopeGuard.
friend ScopeGuard<Callback> MakeScopeGuard<Callback>(Callback&& callback) noexcept;

    explicit ScopeGuard(Callback&& callback) noexcept
        :_callback(std::forward<Callback>(callback))
        , _dismiss(false) {}

private:
    Callback _callback;
    bool _dismiss;
};

// The MakeScopeGuard() function is used to create a new ScopeGuard object.
// It can be instantiated with a lambda function, a std::function<void()>,
// a functor, or a void(*)() function pointer.
template<typename Callback>
ScopeGuard<Callback> MakeScopeGuard(Callback&& callback) noexcept {
    return ScopeGuard<Callback>{ std::forward<Callback>(callback)};
}

namespace internal {
// for TURBO_SCOPE_EXIT.
enum class ScopeExitHelper {};

template<typename Callback>
ScopeGuard<Callback>
operator+(ScopeExitHelper, Callback&& callback) {
    return MakeScopeGuard(std::forward<Callback>(callback));
}
} // namespace internal
} // namespace turbo

#define TURBO_ANONYMOUS_VARIABLE(prefix) TURBO_CONCAT(prefix, __COUNTER__)

// The code in the braces of TURBO_SCOPE_EXIT always executes at the end of the scope.
// Variables used within TURBO_SCOPE_EXIT are captured by reference.
// Example:
// int fd = open(...);
// TURBO_SCOPE_EXIT {
//     close(fd);
// };
// use fd ...
//
#define TURBO_SCOPE_EXIT                                     \
  auto TURBO_ANONYMOUS_VARIABLE(SCOPE_EXIT) =                \
      ::turbo::internal::ScopeExitHelper() + [&]() noexcept
