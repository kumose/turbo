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

find_package(GTest REQUIRED)
find_path(GTEST_INCLUDE_PATH gtest/gtest.h)
include_directories(${GTEST_INCLUDE_PATH})

add_subdirectory(algorithm)
add_subdirectory(base)
add_subdirectory(cleanup)
add_subdirectory(container)
add_subdirectory(crypto)
add_subdirectory(debugging)
add_subdirectory(flags)
add_subdirectory(functional)
add_subdirectory(hash)
add_subdirectory(log)
add_subdirectory(memory)
add_subdirectory(meta)
add_subdirectory(numeric)
add_subdirectory(profile)
add_subdirectory(random)
add_subdirectory(strings)
add_subdirectory(synchronization)
add_subdirectory(times)
add_subdirectory(utility)
add_subdirectory(bootstrap)
add_subdirectory(threading)
add_subdirectory(files)