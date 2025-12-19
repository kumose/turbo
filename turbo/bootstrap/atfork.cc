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

#include <turbo/bootstrap/atfork.h>
#include <turbo/utility/status.h>
#include <algorithm>
#include <atomic>
#include <mutex>
#include <vector>
#include <turbo/log/logging.h>

#ifndef _WIN32

#include <pthread.h>

#endif


namespace turbo {

    namespace {

        // Singleton state for at-fork management.
        // We do not use global variables because of initialization order issues (ARROW-18383).
        // Instead, a function-local static ensures the state is initialized
        // opportunistically (see GetAtForkState()).
        struct AtForkState {
            struct RunningHandler {
                // A temporary owning copy of a handler, to make sure that a handler
                // that runs before fork can still run after fork.
                std::shared_ptr<AtForkHandler> handler;
                // The token returned by the before-fork handler, to pass to after-fork handlers.
                std::any token;

                explicit RunningHandler(std::shared_ptr<AtForkHandler> handler)
                        : handler(std::move(handler)) {}
            };

            void MaintainHandlersUnlocked() {
                auto it = std::remove_if(
                        handlers_.begin(), handlers_.end(),
                        [](const std::weak_ptr<AtForkHandler> &ptr) { return ptr.expired(); });
                handlers_.erase(it, handlers_.end());
            }

            void BeforeFork() {
                // Lock the mutex and keep it locked until the end of AfterForkParent(),
                // to avoid multiple concurrent forks and atforks.
                mutex_.lock();

                DKCHECK(handlers_while_forking_.empty());  // AfterForkParent clears it

                for (const auto &weak_handler: handlers_) {
                    if (auto handler = weak_handler.lock()) {
                        handlers_while_forking_.emplace_back(std::move(handler));
                    }
                }

                // XXX can the handler call RegisterAtFork()?
                for (auto &&handler: handlers_while_forking_) {
                    if (handler.handler->before) {
                        handler.token = handler.handler->before();
                    }
                }
            }

            void AfterForkParent() {
                // The mutex was locked by BeforeFork()
                auto handlers = std::move(handlers_while_forking_);
                handlers_while_forking_.clear();

                // Execute handlers in reverse order
                for (auto it = handlers.rbegin(); it != handlers.rend(); ++it) {
                    auto &&handler = *it;
                    if (handler.handler->parent_after) {
                        handler.handler->parent_after(std::move(handler.token));
                    }
                }

                mutex_.unlock();
                // handlers will be destroyed here without the mutex locked, so that
                // any action taken by destructors might call RegisterAtFork
            }

            void AfterForkChild() {
                // Need to reinitialize the mutex as it is probably invalid.  Also, the
                // old mutex destructor may fail.
                // Fortunately, we are a single thread in the child process by now, so no
                // additional synchronization is needed.
                new(&mutex_) std::mutex;

                auto handlers = std::move(handlers_while_forking_);
                handlers_while_forking_.clear();

                // Execute handlers in reverse order
                for (auto it = handlers.rbegin(); it != handlers.rend(); ++it) {
                    auto &&handler = *it;
                    if (handler.handler->child_after) {
                        handler.handler->child_after(std::move(handler.token));
                    }
                }
            }

            void RegisterAtFork(std::weak_ptr<AtForkHandler> weak_handler) {
                std::lock_guard<std::mutex> lock(mutex_);
                // This is O(n) for each at-fork registration. We assume that n remains
                // typically low and calls to this function are not performance-critical.
                MaintainHandlersUnlocked();
                handlers_.push_back(std::move(weak_handler));
            }

            std::mutex mutex_;
            std::vector<std::weak_ptr<AtForkHandler>> handlers_;
            std::vector<RunningHandler> handlers_while_forking_;
        };

        AtForkState *GetAtForkState() {
            static std::unique_ptr<AtForkState> state = []() {
                auto state = std::make_unique<AtForkState>();
#ifndef _WIN32
                int r = pthread_atfork(/*prepare=*/[] { GetAtForkState()->BeforeFork(); },
                        /*parent=*/[] { GetAtForkState()->AfterForkParent(); },
                        /*child=*/[] { GetAtForkState()->AfterForkChild(); });
                if (r != 0) {
                    turbo::make_status(r, "Error when calling pthread_atfork: ").abort();
                }
#endif
                return state;
            }();
            return state.get();
        }

    };  // namespace

    void register_at_fork(std::weak_ptr<AtForkHandler> weak_handler) {
        GetAtForkState()->RegisterAtFork(std::move(weak_handler));
    }

}  // namespace turbo
