// Copyright (C) 2024 Kumo group inc.
// Author: Jeff.li lijippy@163.com
// All rights reserved.
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
#pragma once

#include <cassert>
#include <initializer_list>
#include <regex>
#include <string_view>
#include <type_traits>

namespace turbo {
    /// Match regex against target and produce string_views out of matches.
    inline bool regex_match(const std::regex& regex, std::string_view target,
                           std::initializer_list<std::string_view*> out_matches) {
      assert(regex.mark_count() == out_matches.size());

      std::match_results<decltype(target.begin())> match;
      if (!std::regex_match(target.begin(), target.end(), match, regex)) {
        return false;
      }

      // Match #0 is the whole matched sequence
      assert(regex.mark_count() + 1 == match.size());
      auto out_it = out_matches.begin();
      for (size_t i = 1; i < match.size(); ++i) {
        **out_it++ = target.substr(match.position(i), match.length(i));
      }
      return true;
    }

}  // namespace turbo

