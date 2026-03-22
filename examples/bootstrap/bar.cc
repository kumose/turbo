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
// Created by jeff on 24-6-1.
//

#include <turbo/bootstrap/boot.h>
#include <iostream>

namespace turbo {

    struct Bar {
        Bar() {
            std::cout << "Bar::Bar" << std::endl;
        }
        ~Bar() {
            std::cout << "Bar::~Bar" << std::endl;
        }
    };

    Bar *g_bar = nullptr;
    struct BarTask : public turbo::BootTask {
        BarTask() = default;
        ~BarTask() override = default;

        void run_boot() override {
            std::cout << "Bar::run_boot" << std::endl;
            g_bar = new Bar();
        }

        void run_shutdown() override {
            std::cout << "Bar::run_shutdown" << std::endl;
            delete g_bar;
        }
    };

    struct BarRegistration {
        BarRegistration() {
            std::cout << "BarRegistration::BarRegistration" << std::endl;
            register_boot_task(std::make_unique<BarTask>(), DEFAULT_BOOT_TASK_PRIORITY);
        }
        ~BarRegistration() {
        }
    };

    BarRegistration g_bar_registration;
}  // namespace turbo