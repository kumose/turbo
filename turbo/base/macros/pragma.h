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
// Created by jeff on 24-6-29.
//

#pragma once

#if defined(__GNUC__)  // GCC or clang
#define TURBO_DIAGNOSTIC_PUSH _Pragma("GCC diagnostic push")
#define TURBO_DIAGNOSTIC_POP _Pragma("GCC diagnostic pop")

#define TURBO_DIAGNOSTIC_IGNORE_DEPRECATED _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")

#elif defined(_MSC_VER)
#define TURBO_DIAGNOSTIC_PUSH __pragma(warning(push))
#define TURBO_DIAGNOSTIC_POP __pragma(warning(pop))

#define TURBO_DIAGNOSTIC_IGNORE_DEPRECATED __pragma(warning(disable : 4996))

#else
#define TURBO_DIAGNOSTIC_PUSH
#define TURBO_DIAGNOSTIC_POP

#define TURBO_DIAGNOSTIC_IGNORE_DEPRECATED

#endif