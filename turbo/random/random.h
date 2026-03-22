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
// -----------------------------------------------------------------------------
// File: random.h
// -----------------------------------------------------------------------------
//
// This header defines the recommended Uniform Random Bit Generator (URBG)
// types for use within the Turbo Random library. These types are not
// suitable for security-related use-cases, but should suffice for most other
// uses of generating random values.
//
// The Turbo random library provides the following URBG types:
//
//   * BitGen, a good general-purpose bit generator, optimized for generating
//     random (but not cryptographically secure) values
//   * InsecureBitGen, a slightly faster, though less random, bit generator, for
//     cases where the existing BitGen is a drag on performance.

#pragma once

#include <turbo/random/random_inl.h>
#include <turbo/random/generator.h>