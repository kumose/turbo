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

#include <string>

#include <turbo/base/macros.h>
#include <turbo/strings/string_view.h>

// --------------------------------------------------------------------
// Program name

namespace turbo::flags_internal {

    // Returns program invocation name or "UNKNOWN" if `SetProgramInvocationName()`
    // is never called. At the moment this is always set to argv[0] as part of
    // library initialization.
    std::string ProgramInvocationName();

    // Returns base name for program invocation name. For example, if
    //   ProgramInvocationName() == "a/b/mybinary"
    // then
    //   ShortProgramInvocationName() == "mybinary"
    std::string ShortProgramInvocationName();

    // Sets program invocation name to a new value. Should only be called once
    // during program initialization, before any threads are spawned.
    void SetProgramInvocationName(std::string_view prog_name_str);

}  // namespace turbo::flags_internal
