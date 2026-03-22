# Copyright (C) Kumo inc. and its affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

#[[
kmcmake_cc_binary(
        NAME hashes_example
        SOURCES hashes_example.cc
        DEPS tedis::tedis ${KMCMAKE_DEPS_LINK}
        COPTS ${USER_CXX_FLAGS}
)
]]
#[[
kmcmake_cc_binary(
        NAME foo_ex
        SOURCES foo_ex.cc
        DEPS changeme::foo ${KMCMAKE_DEPS_LINK}
        COPTS ${USER_CXX_FLAGS}
)
]]
add_subdirectory(log)
add_subdirectory(times)
add_subdirectory(status)
add_subdirectory(bootstrap)
add_subdirectory(flags)
add_subdirectory(sync)