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

#include <turbo/log/initialize.h>

#include <turbo/base/macros.h>
#include <turbo/log/internal/globals.h>
#include <turbo/times/time.h>

namespace turbo {
    TURBO_NAMESPACE_BEGIN

    namespace {
        void InitializeLogImpl(turbo::TimeZone time_zone) {
            // This comes first since it is used by RAW_LOG.
            turbo::log_internal::SetTimeZone(time_zone);

            // Note that initialization is complete, so logs can now be sent to their
            // proper destinations rather than stderr.
            log_internal::SetInitialized();
        }
    } // namespace

    void initialize_log() { InitializeLogImpl(turbo::TimeZone::local()); }

    TURBO_NAMESPACE_END
} // namespace turbo
