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

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <turbo/container/flat_hash_map.h>
#include <utility>
#include <vector>

#include <turbo/functional/function_ref.h>
#include <turbo/utility/status.h>
#include <turbo/base/macros.h>

namespace turbo {

    /// \brief A container for key-value pair type metadata. Not thread-safe
    class TURBO_EXPORT KeyValueMetadata {
    public:
        KeyValueMetadata();

        KeyValueMetadata(std::vector<std::string> keys, std::vector<std::string> values);

        explicit KeyValueMetadata(const std::unordered_map<std::string, std::string> &map);

        static std::shared_ptr<KeyValueMetadata> create(std::vector<std::string> keys,
                                                      std::vector<std::string> values);

        void mapping(std::unordered_map<std::string, std::string> *out) const;

        void append(std::string key, std::string value);

        turbo::Result<std::string> get(std::string_view key) const;

        bool contains(std::string_view key) const;

        // Note that deleting may invalidate known indices
        turbo::Status remove(std::string_view key);

        turbo::Status remove(int64_t index);

        turbo::Status remove_many(std::vector<int64_t> indices);

        turbo::Status set(std::string key, std::string value);

        void reserve(int64_t n);

        int64_t size() const;

        const std::string &key(int64_t i) const;

        const std::string &value(int64_t i) const;

        const std::vector<std::string> &keys() const { return keys_; }

        const std::vector<std::string> &values() const { return values_; }

        std::vector<std::pair<std::string, std::string>> sorted_pairs() const;

        /// \brief Perform linear search for key, returning -1 if not found
        int find_key(std::string_view key) const;

        std::shared_ptr<KeyValueMetadata> copy() const;

        /// \brief Return a new KeyValueMetadata by combining the passed metadata
        /// with this KeyValueMetadata. Colliding keys will be overridden by the
        /// passed metadata. Assumes keys in both containers are unique
        std::shared_ptr<KeyValueMetadata> merge(const KeyValueMetadata &other) const;

        bool equals(const KeyValueMetadata &other) const;

        std::string to_string() const;

    private:
        std::vector<std::string> keys_;
        std::vector<std::string> values_;

        TURBO_DISALLOW_COPY_AND_ASSIGN(KeyValueMetadata);
    };

    /// \brief Create a KeyValueMetadata instance
    ///
    /// \param pairs key-value mapping
    TURBO_EXPORT std::shared_ptr<KeyValueMetadata> key_value_metadata(
            const std::unordered_map<std::string, std::string> &pairs);

    /// \brief Create a KeyValueMetadata instance
    ///
    /// \param keys sequence of metadata keys
    /// \param values sequence of corresponding metadata values
    TURBO_EXPORT std::shared_ptr<KeyValueMetadata> key_value_metadata(
            std::vector<std::string> keys, std::vector<std::string> values);

}  // namespace turbo
