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

    struct Foo {
        Foo() {
            std::cout << "Foo::Foo" << std::endl;
        }
        ~Foo() {
            std::cout << "Foo::~Foo" << std::endl;
        }
    };

    Foo *g_foo = nullptr;
    struct FooTask : public turbo::BootTask {
        FooTask() = default;
        ~FooTask() override = default;

        void run_boot() override {
            std::cout << "Foo::run_boot" << std::endl;
            g_foo = new Foo();
        }

        void run_shutdown() override {
            std::cout << "Foo::run_shutdown" << std::endl;
            delete g_foo;
        }
    };

    struct FooRegistration {
        FooRegistration() {
            std::cout << "FooRegistration::FooRegistration" << std::endl;
            register_boot_task(std::make_unique<FooTask>(), DEFAULT_BOOT_TASK_PRIORITY);
        }
        ~FooRegistration() {
        }
    };

    TURBO_DLL static FooRegistration g_foo_registration __attribute__((used)) = FooRegistration();
}  // namespace turbo