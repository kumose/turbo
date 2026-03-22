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

kmcmake_cc_test_library(
        NAMESPACE turbo
        NAME distribution_test_util
        SOURCES distribution_test_util.h distribution_test_util.cc
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)

kmcmake_cc_test_library(
        NAMESPACE turbo
        NAME chi_square
        SOURCES chi_square.cc chi_square.h
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)

set(RANDOM_TEST_SRC
        bernoulli_distribution_test
        beta_distribution_test
        bit_gen_ref_test
        chi_square_test
        discrete_distribution_test
        distributions_test
        distribution_test_util_test
        examples_test
        explicit_seed_seq_test
        exponential_distribution_test
        fastmath_test
        fast_uniform_bits_test
        gaussian_distribution_test
        generate_real_test
        generators_test
        iostream_state_saver_test
        log_uniform_int_distribution_test
        mock_distributions_test
        mocking_bit_gen_test
        nanobenchmark_test
        nonsecure_base_test
        pcg_engine_test
        poisson_distribution_test
        pool_urbg_test
        randen_engine_test
        randen_hwaes_test
        randen_slow_test
        randen_test
        seed_sequences_test
        salted_seed_seq_test
        seed_material_test
        traits_test
        uniform_helper_test
        uniform_int_distribution_test
        uniform_real_distribution_test
        wide_multiply_test
        zipf_distribution_test
)
foreach (test ${RANDOM_TEST_SRC})

    kmcmake_cc_test(
            NAME ${test}
            MODULE random
            SOURCES ${test}.cc
            LINKS
            turbo::turbo
            chi_square_static
            distribution_test_util_static
            GTest::gtest
            GTest::gmock
            GTest::gmock_main
            ${KMCMAKE_DEPS_LINK}
            CXXOPTS ${KMCMAKE_CXX_OPTIONS}
    )
endforeach ()

kmcmake_cc_test(
        NAME rand_util_test
        MODULE random
        SOURCES rand_util_test.cc
        DEPS
        test_instance_tracker_static
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        GTest::gmock_main
        ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)