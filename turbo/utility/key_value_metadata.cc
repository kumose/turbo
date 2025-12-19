// Copyright (C) 2024 Kumo inc.
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

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <turbo/functional/iterator.h>
#include <turbo/utility/key_value_metadata.h>
#include <turbo/log/logging.h>
#include <turbo/algorithm/algorithm.h>

using std::size_t;

namespace turbo {

    static std::vector<std::string> UnorderedMapKeys(
        const std::unordered_map<std::string, std::string>& map) {
      std::vector<std::string> keys;
      keys.reserve(map.size());
      for (const auto& pair : map) {
        keys.push_back(pair.first);
      }
      return keys;
    }

    static std::vector<std::string> UnorderedMapValues(
        const std::unordered_map<std::string, std::string>& map) {
      std::vector<std::string> values;
      values.reserve(map.size());
      for (const auto& pair : map) {
        values.push_back(pair.second);
      }
      return values;
    }

    KeyValueMetadata::KeyValueMetadata() {}

    KeyValueMetadata::KeyValueMetadata(
        const std::unordered_map<std::string, std::string>& map)
        : keys_(UnorderedMapKeys(map)), values_(UnorderedMapValues(map)) {
      KCHECK_EQ(keys_.size(), values_.size());
    }

    KeyValueMetadata::KeyValueMetadata(std::vector<std::string> keys,
                                       std::vector<std::string> values)
        : keys_(std::move(keys)), values_(std::move(values)) {
        KCHECK_EQ(keys.size(), values.size());
    }

    std::shared_ptr<KeyValueMetadata> KeyValueMetadata::create(
        std::vector<std::string> keys, std::vector<std::string> values) {
      return std::make_shared<KeyValueMetadata>(std::move(keys), std::move(values));
    }

    void KeyValueMetadata::mapping(std::unordered_map<std::string, std::string> *out) const {
      DKCHECK_NE(out, nullptr);
      const int64_t n = size();
      out->reserve(n);
      for (int64_t i = 0; i < n; ++i) {
        out->insert(std::make_pair(key(i), value(i)));
      }
    }

    void KeyValueMetadata::append(std::string key, std::string value) {
      keys_.push_back(std::move(key));
      values_.push_back(std::move(value));
    }

    turbo::Result<std::string> KeyValueMetadata::get(std::string_view key) const {
      auto index = find_key(key);
      if (index < 0) {
        return turbo::not_found_error(key);
      } else {
        return value(index);
      }
    }

    turbo::Status KeyValueMetadata::remove(int64_t index) {
      keys_.erase(keys_.begin() + index);
      values_.erase(values_.begin() + index);
      return turbo::OkStatus();
    }

    turbo::Status KeyValueMetadata::remove_many(std::vector<int64_t> indices) {
      std::sort(indices.begin(), indices.end());
      const int64_t size = static_cast<int64_t>(keys_.size());
      indices.push_back(size);

      int64_t shift = 0;
      for (int64_t i = 0; i < static_cast<int64_t>(indices.size() - 1); ++i) {
        ++shift;
        const auto start = indices[i] + 1;
        const auto stop = indices[i + 1];
        DKCHECK_GE(start, 0);
        DKCHECK_LE(start, size);
        DKCHECK_GE(stop, 0);
        DKCHECK_LE(stop, size);
        for (int64_t index = start; index < stop; ++index) {
          keys_[index - shift] = std::move(keys_[index]);
          values_[index - shift] = std::move(values_[index]);
        }
      }
      keys_.resize(size - shift);
      values_.resize(size - shift);
      return turbo::OkStatus();
    }

    turbo::Status KeyValueMetadata::remove(std::string_view key) {
      auto index = find_key(key);
      if (index < 0) {
        return turbo::not_found_error(key);
      } else {
        return remove(index);
      }
    }

    turbo::Status KeyValueMetadata::set(std::string key, std::string value) {
      auto index = find_key(key);
      if (index < 0) {
        append(std::move(key), std::move(value));
      } else {
        keys_[index] = std::move(key);
        values_[index] = std::move(value);
      }
      return turbo::OkStatus();
    }

    bool KeyValueMetadata::contains(std::string_view key) const { return find_key(key) >= 0; }

    void KeyValueMetadata::reserve(int64_t n) {
      DKCHECK_GE(n, 0);
      const auto m = static_cast<size_t>(n);
      keys_.reserve(m);
      values_.reserve(m);
    }

    int64_t KeyValueMetadata::size() const {
      DKCHECK_EQ(keys_.size(), values_.size());
      return static_cast<int64_t>(keys_.size());
    }

    const std::string& KeyValueMetadata::key(int64_t i) const {
      DKCHECK_GE(i, 0);
      DKCHECK_LT(static_cast<size_t>(i), keys_.size());
      return keys_[i];
    }

    const std::string& KeyValueMetadata::value(int64_t i) const {
      DKCHECK_GE(i, 0);
      DKCHECK_LT(static_cast<size_t>(i), values_.size());
      return values_[i];
    }

    std::vector<std::pair<std::string, std::string>> KeyValueMetadata::sorted_pairs() const {
      std::vector<std::pair<std::string, std::string>> pairs;
      pairs.reserve(size());

      auto indices = turbo::arg_sort(keys_);
      for (const auto i : indices) {
        pairs.emplace_back(keys_[i], values_[i]);
      }
      return pairs;
    }

    int KeyValueMetadata::find_key(std::string_view key) const {
      for (size_t i = 0; i < keys_.size(); ++i) {
        if (keys_[i] == key) {
          return static_cast<int>(i);
        }
      }
      return -1;
    }

    std::shared_ptr<KeyValueMetadata> KeyValueMetadata::copy() const {
      return std::make_shared<KeyValueMetadata>(keys_, values_);
    }

    std::shared_ptr<KeyValueMetadata> KeyValueMetadata::merge(
        const KeyValueMetadata& other) const {
      std::unordered_set<std::string> observed_keys;
      std::vector<std::string> result_keys;
      std::vector<std::string> result_values;

      result_keys.reserve(keys_.size());
      result_values.reserve(keys_.size());

      for (int64_t i = 0; i < other.size(); ++i) {
        const auto& key = other.key(i);
        auto it = observed_keys.find(key);
        if (it == observed_keys.end()) {
          result_keys.push_back(key);
          result_values.push_back(other.value(i));
          observed_keys.insert(key);
        }
      }
      for (size_t i = 0; i < keys_.size(); ++i) {
        auto it = observed_keys.find(keys_[i]);
        if (it == observed_keys.end()) {
          result_keys.push_back(keys_[i]);
          result_values.push_back(values_[i]);
          observed_keys.insert(keys_[i]);
        }
      }

      return std::make_shared<KeyValueMetadata>(std::move(result_keys),
                                                std::move(result_values));
    }

    bool KeyValueMetadata::equals(const KeyValueMetadata& other) const {
      if (size() != other.size()) {
        return false;
      }

      auto indices = turbo::arg_sort(keys_);
      auto other_indices = turbo::arg_sort(other.keys_);

      for (int64_t i = 0; i < size(); ++i) {
        auto j = indices[i];
        auto k = other_indices[i];
        if (keys_[j] != other.keys_[k] || values_[j] != other.values_[k]) {
          return false;
        }
      }
      return true;
    }

    std::string KeyValueMetadata::to_string() const {
      std::stringstream buffer;

      buffer << "\n-- metadata --";
      for (int64_t i = 0; i < size(); ++i) {
        buffer << "\n" << keys_[i] << ": " << values_[i];
      }

      return buffer.str();
    }

    std::shared_ptr<KeyValueMetadata> key_value_metadata(
        const std::unordered_map<std::string, std::string>& pairs) {
      return std::make_shared<KeyValueMetadata>(pairs);
    }

    std::shared_ptr<KeyValueMetadata> key_value_metadata(std::vector<std::string> keys,
                                                         std::vector<std::string> values) {
      return std::make_shared<KeyValueMetadata>(std::move(keys), std::move(values));
    }

}  // namespace turbo
