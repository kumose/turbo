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
#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>
#include <tuple>
#include <utility>

#include <turbo/container/htrie_set.h>
#include "utils.h"

BOOST_AUTO_TEST_SUITE(test_htrie_set)

using test_types = boost::mpl::list<turbo::htrie_set<char>>;

/**
 * insert
 */
BOOST_AUTO_TEST_CASE_TEMPLATE(test_insert, TMap, test_types) {
  // insert x values, insert them again, check values
  using char_tt = typename TMap::char_type;

  const std::size_t nb_values = 100000;
  TMap set;
  typename TMap::iterator it;
  bool inserted;

  for (std::size_t i = 0; i < nb_values; i++) {
    std::tie(it, inserted) = set.insert(utils::get_key<char_tt>(i));

    BOOST_CHECK_EQUAL(it.key(), (utils::get_key<char_tt>(i)));
    BOOST_CHECK(inserted);
  }
  BOOST_CHECK_EQUAL(set.size(), nb_values);

  for (std::size_t i = 0; i < nb_values; i++) {
    std::tie(it, inserted) = set.insert(utils::get_key<char_tt>(i));

    BOOST_CHECK_EQUAL(it.key(), (utils::get_key<char_tt>(i)));
    BOOST_CHECK(!inserted);
  }

  for (std::size_t i = 0; i < nb_values; i++) {
    it = set.find(utils::get_key<char_tt>(i));

    BOOST_CHECK(it != set.end());
    BOOST_CHECK_EQUAL(it.key(), (utils::get_key<char_tt>(i)));
  }

  for (auto it = set.begin(); it != set.end(); ++it) {
    auto it_find = set.find(it.key());

    BOOST_CHECK(it_find != set.end());
    BOOST_CHECK_EQUAL(it_find.key(), it.key());
  }
}

/**
 * operator=
 */
BOOST_AUTO_TEST_CASE(test_assign_operator) {
  turbo::htrie_set<char> set = {"test1", "test2"};
  BOOST_CHECK_EQUAL(set.size(), 2);

  set = {"test3"};
  BOOST_CHECK_EQUAL(set.size(), 1);
  BOOST_CHECK_EQUAL(set.count("test3"), 1);
}

BOOST_AUTO_TEST_CASE(test_copy_operator) {
  turbo::htrie_set<char> set = {"test1", "test2", "test3", "test4"};
  turbo::htrie_set<char> set2 = set;
  turbo::htrie_set<char> set3;
  set3 = set;

  BOOST_CHECK(set == set2);
  BOOST_CHECK(set == set3);
}

BOOST_AUTO_TEST_CASE(test_move_operator) {
  turbo::htrie_set<char> set = {"test1", "test2"};
  turbo::htrie_set<char> set2 = std::move(set);

  BOOST_CHECK(set.empty());
  BOOST_CHECK(set.begin() == set.end());
  BOOST_CHECK_EQUAL(set2.size(), 2);
  BOOST_CHECK(set2 == (turbo::htrie_set<char>{"test1", "test2"}));

  turbo::htrie_set<char> set3;
  set3 = std::move(set2);

  BOOST_CHECK(set2.empty());
  BOOST_CHECK(set2.begin() == set2.end());
  BOOST_CHECK_EQUAL(set3.size(), 2);
  BOOST_CHECK(set3 == (turbo::htrie_set<char>{"test1", "test2"}));

  set2 = {"test1"};
  BOOST_CHECK(set2 == (turbo::htrie_set<char>{"test1"}));
}

/**
 * serialize and deserialize
 */
BOOST_AUTO_TEST_CASE(test_serialize_deserialize) {
  // insert x values; delete some values; serialize set; deserialize in new set;
  // check equal. for deserialization, test it with and without hash
  // compatibility.
  const std::size_t nb_values = 1000;

  turbo::htrie_set<char> set(0);

  set.insert("");
  for (std::size_t i = 1; i < nb_values + 40; i++) {
    set.insert(utils::get_key<char>(i));
  }

  for (std::size_t i = nb_values; i < nb_values + 40; i++) {
    set.erase(utils::get_key<char>(i));
  }
  BOOST_CHECK_EQUAL(set.size(), nb_values);

  serializer serial;
  set.serialize(serial);

  deserializer dserial(serial.str());
  auto set_deserialized = decltype(set)::deserialize(dserial, true);
  BOOST_CHECK(set_deserialized == set);

  deserializer dserial2(serial.str());
  set_deserialized = decltype(set)::deserialize(dserial2, false);
  BOOST_CHECK(set_deserialized == set);
}

BOOST_AUTO_TEST_SUITE_END()
