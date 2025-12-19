// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

// approximate quantiles from arbitrary length dataset with O(1) space
// based on 'Computing Extremely Accurate Quantiles Using t-Digests' from Dunning & Ertl
// - https://arxiv.org/abs/1902.04023
// - https://github.com/tdunning/t-digest

#pragma once

#include <cmath>
#include <memory>
#include <vector>
#include <turbo/base/macros.h>
#include <turbo/log/logging.h>
#include <turbo/utility/status.h>

namespace turbo {

    class TURBO_EXPORT TDigest {
    public:
        explicit TDigest(uint32_t delta = 100, uint32_t buffer_size = 500);

        ~TDigest();

        TDigest(TDigest &&);

        TDigest &operator=(TDigest &&);

        // reset and re-use this tdigest
        void reset();

        // validate data integrity
        Status validate() const;

        // dump internal data, only for debug
        void dump() const;

        // buffer a single data point, consume internal buffer if full
        // this function is intensively called and performance critical
        // call it only if you are sure no NAN exists in input data
        void add(double value) {
            DKCHECK(!std::isnan(value)) << "cannot add NAN";
            if (TURBO_UNLIKELY(input_.size() == input_.capacity())) {
                merge_input();
            }
            input_.push_back(value);
        }

        // skip NAN on adding
        template<typename T>
        typename std::enable_if<std::is_floating_point<T>::value>::type nan_add(T value) {
            if (!std::isnan(value)) {
                add(value);
            }
        }

        template<typename T>
        typename std::enable_if<std::is_integral<T>::value>::type nan_add(T value) {
            add(static_cast<double>(value));
        }

        // merge with other t-digests, called infrequently
        void merge(const std::vector<TDigest> &others);

        void merge(const TDigest &other);

        // calculate quantile
        double quantile(double q) const;

        double min() const { return quantile(0); }

        double max() const { return quantile(1); }

        double mean() const;

        // check if this tdigest contains no valid data points
        bool is_empty() const;

    private:
        // merge input data with current tdigest
        void merge_input() const;

        // input buffer, size = buffer_size * sizeof(double)
        mutable std::vector<double> input_;

        // hide other members with pimpl
        class TDigestImpl;

        std::unique_ptr<TDigestImpl> impl_;
    };

}  // namespace turbo

