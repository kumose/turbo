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

#ifndef TURBO_USE_STATIC_RTTI
#if(defined(_HAS_STATIC_RTTI) && _HAS_STATIC_RTTI)
#define TURBO_USE_STATIC_RTTI 1
#elif defined(__cpp_rtti)
#if(defined(_CPPRTTI) && _CPPRTTI == 0)
#define TURBO_USE_STATIC_RTTI 1
#else
#define TURBO_USE_STATIC_RTTI 0
#endif
#elif(defined(__GCC_RTTI) && __GXX_RTTI)
#define TURBO_USE_STATIC_RTTI 0
#else
#define TURBO_USE_STATIC_RTTI 1
#endif
#endif
