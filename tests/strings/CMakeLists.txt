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


set(STRINGS_TEST_SRC
        arg_test
        ascii_test
        #atod_manual_test
        bind_test
        char_formatting_test
        charconv_bigint_test
        charconv_parse_test
        charconv_test
        charset_test
        checker_test
        convert_test
        cord_buffer_test
        cord_data_edge_test
        cord_rep_btree_navigator_test
        cord_rep_btree_reader_test
        cord_rep_btree_test
        cord_rep_crc_test
        cord_test
        cordz_functions_test
        cordz_handle_test
        cordz_info_statistics_test
        cordz_info_test
        cordz_sample_token_test
        cordz_test
        cordz_update_scope_test
        cordz_update_tracker_test
        damerau_levenshtein_distance_test
        encoding_test
        escaping_test
        extension_test
        has_stringify_test
        has_ostream_operator_test
        match_test
        memutil_test
        numbers_test
        ostringstream_test
        output_test
        parser_test
        pow10_helper_test
        resize_uninitialized_test
        small_string_test
        str_cat_test
        str_format_test
        str_join_test
        str_replace_test
        str_split_test
        string_constant_test
        string_view_test
        strip_test
        substitute_test
        utf8_test
)
foreach (test ${STRINGS_TEST_SRC})

    kmcmake_cc_test(
            NAME ${test}
            MODULE strings
            SOURCES ${test}.cc
            LINKS
            turbo::turbo
            time_test_util_static
            GTest::gtest
            GTest::gmock
            GTest::gmock_main
            ${KMCMAKE_DEPS_LINK}
            CXXOPTS ${KMCMAKE_CXX_OPTIONS}
    )
endforeach ()