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

#include <turbo/strings/str_replace.h>

#include <cstddef>
#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

#include <turbo/base/macros.h>
#include <turbo/base/nullability.h>
#include <turbo/strings/str_cat.h>
#include <turbo/strings/string_view.h>

namespace turbo {
TURBO_NAMESPACE_BEGIN
namespace strings_internal {

using FixedMapping =
    std::initializer_list<std::pair<std::string_view, std::string_view>>;

// Applies the ViableSubstitutions in subs_ptr to the std::string_view s, and
// stores the result in *result_ptr. Returns the number of substitutions that
// occurred.
int ApplySubstitutions(
    std::string_view s,
    turbo::Nonnull<std::vector<strings_internal::ViableSubstitution>*> subs_ptr,
    turbo::Nonnull<std::string*> result_ptr) {
  auto& subs = *subs_ptr;
  int substitutions = 0;
  size_t pos = 0;
  while (!subs.empty()) {
    auto& sub = subs.back();
    if (sub.offset >= pos) {
      if (pos <= s.size()) {
        str_append(result_ptr, s.substr(pos, sub.offset - pos), sub.replacement);
      }
      pos = sub.offset + sub.old.size();
      substitutions += 1;
    }
    sub.offset = s.find(sub.old, pos);
    if (sub.offset == s.npos) {
      subs.pop_back();
    } else {
      // Insertion sort to ensure the last ViableSubstitution continues to be
      // before all the others.
      size_t index = subs.size();
      while (--index && subs[index - 1].OccursBefore(subs[index])) {
        std::swap(subs[index], subs[index - 1]);
      }
    }
  }
  result_ptr->append(s.data() + pos, s.size() - pos);
  return substitutions;
}

}  // namespace strings_internal

// We can implement this in terms of the generic str_replace_all, but
// we must specify the template overload because C++ cannot deduce the type
// of an initializer_list parameter to a function, and also if we don't specify
// the type, we just call ourselves.
//
// Note that we implement them here, rather than in the header, so that they
// aren't inlined.

std::string str_replace_all(std::string_view s,
                          strings_internal::FixedMapping replacements) {
  return str_replace_all<strings_internal::FixedMapping>(s, replacements);
}

int str_replace_all(strings_internal::FixedMapping replacements,
                  turbo::Nonnull<std::string*> target) {
  return str_replace_all<strings_internal::FixedMapping>(replacements, target);
}

TURBO_NAMESPACE_END
}  // namespace turbo
