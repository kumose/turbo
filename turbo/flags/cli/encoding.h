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

#pragma once
#include <turbo/base/macros.h>
#include <string>
#include <string_view>

namespace turbo::cli {


/// Convert a wide string to a narrow string.
std::string narrow(const std::wstring &str);
std::string narrow(const wchar_t *str);
std::string narrow(const wchar_t *str, std::size_t size);

/// Convert a narrow string to a wide string.
std::wstring widen(const std::string &str);
std::wstring widen(const char *str);
std::wstring widen(const char *str, std::size_t size);

std::string narrow(std::wstring_view str);
std::wstring widen(std::string_view str);

}  // namespace turbo::cli
