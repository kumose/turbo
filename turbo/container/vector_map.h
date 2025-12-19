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

#include <memory>
#include <functional>
#include <vector>
#include <utility>
#include <algorithm>
#include <initializer_list>
#include <stddef.h>
#include <stdexcept>
#include <iterator>

namespace turbo {


    /// map_value_compare
    ///
    /// Our adapter for the comparison function in the template parameters.
    ///
    template<typename Key, typename Value, typename Compare>
    class map_value_compare : public Compare {
    public:
        explicit map_value_compare(const Compare &x)
                : Compare(x) {}

        bool operator()(const Value &a, const Value &b) const { return Compare::operator()(a.first, b.first); }

        bool operator()(const Value &a, const Key &b) const { return Compare::operator()(a.first, b); }

        bool operator()(const Key &a, const Value &b) const { return Compare::operator()(a, b.first); }

        bool operator()(const Key &a, const Key &b) const { return Compare::operator()(a, b); }

    }; // map_value_compare



    /// vector_map
    ///
    /// Implements a map via a random access container such as a vector.
    ///
    /// Note that with vector_set, vector_multiset, vector_map, vector_multimap
    /// that the modification of the container potentially invalidates all
    /// existing iterators into the container, unlike what happens with conventional
    /// sets and maps.
    ///
    /// This type could conceptually use a std::array as its underlying container,
    /// however the current design requires an allocator aware container.
    /// Consider using a fixed_vector instead.
    ///
    /// Note that we set the value_type to be pair<Key, T> and not pair<const Key, T>.
    /// This means that the underlying container (e.g vector) is a container of pair<Key, T>.
    /// Our vector and deque implementations are optimized to assign values in-place and
    /// using a vector of pair<const Key, T> (note the const) would make it hard to use
    /// our existing vector implementation without a lot of headaches. As a result,
    /// at least for the time being we do away with the const. This means that the
    /// insertion type varies between map and vector_map in that the latter doesn't take
    /// const. This also means that a certain amount of automatic safety provided by
    /// the implementation is lost, as the compiler will let the wayward user modify
    /// a key and thus make the container no longer ordered behind its back.
    ///
    template<typename Key, typename T, typename Compare = std::less<Key>,
            typename RandomAccessContainer = std::vector<std::pair<Key, T>>
    >
    class vector_map : protected map_value_compare<Key, std::pair<Key, T>, Compare> {
    public:
        typedef RandomAccessContainer storage_type;
        typedef vector_map<Key, T, Compare, RandomAccessContainer> this_type;
        typedef typename RandomAccessContainer::allocator_type allocator_type;
        typedef Key key_type;
        typedef T mapped_type;
        typedef std::pair<Key, T> value_type;
        typedef Compare key_compare;
        typedef map_value_compare<Key, value_type, Compare> value_compare;
        typedef value_type *pointer;
        typedef const value_type *const_pointer;
        typedef value_type &reference;
        typedef const value_type &const_reference;
        typedef typename storage_type::size_type size_type;
        typedef typename storage_type::difference_type difference_type;
        typedef typename storage_type::iterator iterator;
        typedef typename storage_type::const_iterator const_iterator;
        typedef typename storage_type::reverse_iterator reverse_iterator;
        typedef typename storage_type::const_reverse_iterator const_reverse_iterator;
        typedef std::pair<iterator, bool> insert_return_type;

    public:
        // We have an empty ctor and a ctor that takes an allocator instead of one for both
        // because this way our RandomAccessContainer wouldn't be required to have an constructor
        // that takes allocator_type.
        vector_map();

        explicit vector_map(const key_compare &comp);

        vector_map(const this_type &x);

        vector_map(this_type &&x);

        vector_map(std::initializer_list<value_type> ilist, const key_compare &compare = key_compare());

        template<typename InputIterator>
        vector_map(InputIterator first,
                   InputIterator last); // allocator arg removed because VC7.1 fails on the default arg. To do: Make a second version of this function without a default arg.

        template<typename InputIterator>
        vector_map(InputIterator first, InputIterator last,
                   const key_compare &compare); // allocator arg removed because VC7.1 fails on the default arg. To do: Make a second version of this function without a default arg.

        this_type &operator=(const this_type &x);

        this_type &operator=(std::initializer_list<value_type> ilist);

        this_type &operator=(this_type &&x);

        this_type &operator=(storage_type &&x);

        this_type &operator=(const storage_type &x);

        void swap(this_type &x);

        const key_compare &key_comp() const;

        key_compare &key_comp();

        const value_compare &value_comp() const;

        value_compare &value_comp();

        // Inherited from base class:
        //
        //     allocator_type& get_allocator();
        //     void            set_allocator(const allocator_type& allocator);
        //
        //     iterator       begin();
        //     const_iterator begin() const;
        //     const_iterator cbegin() const;
        //
        //     iterator       end();
        //     const_iterator end() const;
        //     const_iterator cend() const;
        //
        //     reverse_iterator       rbegin();
        //     const_reverse_iterator rbegin() const;
        //     const_reverse_iterator crbegin() const;
        //
        //     reverse_iterator       rend();
        //     const_reverse_iterator rend() const;
        //     const_reverse_iterator crend() const;
        //
        //     size_type size() const;
        //     bool      empty() const;
        //     void      clear();

        template<class... Args>
        std::pair<iterator, bool> emplace(Args &&... args);

        template<class... Args>
        iterator emplace_hint(const_iterator position, Args &&... args);

        template<typename P, typename = std::enable_if_t<std::is_constructible_v<value_type, P &&>>>
        std::pair<iterator, bool> insert(P &&otherValue);

        std::pair<iterator, bool> insert(const value_type &value);

        std::pair<iterator, bool> insert(const key_type &otherValue);

        std::pair<iterator, bool> insert(key_type &&otherValue);

        iterator insert(const_iterator position, const value_type &value);

        iterator insert(const_iterator position, value_type &&value);

        void insert(std::initializer_list<value_type> ilist);

        template<typename InputIterator>
        void insert(InputIterator first, InputIterator last);

        iterator erase(const_iterator position);

        iterator erase(const_iterator first, const_iterator last);

        size_type erase(const key_type &k);

        reverse_iterator erase(const_reverse_iterator position);

        reverse_iterator erase(const_reverse_iterator first, const_reverse_iterator last);

        iterator find(const key_type &k);

        const_iterator find(const key_type &k) const;

        template<typename U, typename BinaryPredicate>
        iterator find_as(const U &u, BinaryPredicate predicate);

        template<typename U, typename BinaryPredicate>
        const_iterator find_as(const U &u, BinaryPredicate predicate) const;

        size_type count(const key_type &k) const;

        iterator lower_bound(const key_type &k);

        const_iterator lower_bound(const key_type &k) const;

        iterator upper_bound(const key_type &k);

        const_iterator upper_bound(const key_type &k) const;

        std::pair<iterator, iterator> equal_range(const key_type &k);

        std::pair<const_iterator, const_iterator> equal_range(const key_type &k) const;

        template<typename U, typename BinaryPredicate>
        std::pair<iterator, iterator> equal_range(const U &u, BinaryPredicate predicate);

        template<typename U, typename BinaryPredicate>
        std::pair<const_iterator, const_iterator> equal_range(const U &u, BinaryPredicate) const;

        // Note: vector_map operator[] returns a reference to the mapped_type, same as map does.
        // But there's an important difference: This reference can be invalidated by -any- changes
        // to the vector_map that cause it to change capacity. This is unlike map, with which
        // mapped_type references are invalidated only if that mapped_type element itself is removed
        // from the map. This is because vector is array-based and map is node-based. As a result
        // the following code that is safe for map is unsafe for vector_map for the case that
        // the vMap[100] doesn't already exist in the vector_map:
        //     vMap[100] = vMap[0]
        mapped_type &operator[](const key_type &k);

        mapped_type &operator[](key_type &&k);

        // after the deprecation period the above should be replaced with:
        mapped_type &at(const key_type &k) { return at_key(k); }

        const mapped_type &at(const key_type &k) const { return at_key(k); }

        // aka. the standard's at() member function.
        mapped_type &at_key(const key_type &k);

        const mapped_type &at_key(const key_type &k) const;

        // Functions which are disallowed due to being unsafe.
        void push_back(const value_type &value) = delete;

        reference push_back() = delete;

        void *push_back_uninitialized() = delete;

        template<class... Args>
        reference emplace_back(Args &&...) = delete;

        // NOTE(rparolin): It is undefined behaviour if user code fails to ensure the container
        // invariants are respected by performing an explicit call to 'sort' before any other
        // operations on the container are performed that do not clear the elements.
        //
        // 'push_back_unsorted' and 'emplace_back_unsorted' do not satisfy container invariants
        // for being sorted. We provide these overloads explicitly labelled as '_unsorted' as an
        // optimization opportunity when batch inserting elements so users can defer the cost of
        // sorting the container once when all elements are contained. This was done to clarify
        // the intent of code by leaving a trace that a manual call to sort is required.
        //
        template<typename... Args>
        decltype(auto) push_back_unsorted(Args &&... args) { return _data.push_back(std::forward<Args>(args)...); }

        template<typename... Args>
        decltype(auto) emplace_back_unsorted(Args &&... args) {
            return _data.emplace_back(std::forward<Args>(args)...);
        }

    private:
        RandomAccessContainer _data;
        value_compare _cmp;

    }; // vector_map





    ///////////////////////////////////////////////////////////////////////
    // vector_map
    ///////////////////////////////////////////////////////////////////////

    template<typename K, typename T, typename C, typename RAC>
    inline vector_map<K, T, C, RAC>::vector_map()
            : value_compare(C()), _data() {

    }

    template<typename K, typename T, typename C, typename RAC>
    inline vector_map<K, T, C, RAC>::vector_map(const key_compare &comp)
            : value_compare(comp), _data() {
        // Empty
    }


    template<typename K, typename T, typename C, typename RAC>
    inline vector_map<K, T, C, RAC>::vector_map(const this_type &x)
            : value_compare(x.value_comp), _data(x._data) {
        // Empty
    }


    template<typename K, typename T, typename C, typename RAC>
    inline vector_map<K, T, C, RAC>::vector_map(this_type &&x)
    // careful to only copy / move the distinct base sub-objects of x:
            : _cmp(x._cmp), _data(std::move(x._data)) {
        // Empty. Note: x is left with empty contents but its original value_compare instead of the default one.
    }


    template<typename K, typename T, typename C, typename RAC>
    inline vector_map<K, T, C, RAC>::vector_map(std::initializer_list<value_type> ilist, const key_compare &compare)
            : _cmp(compare), _data() {
        insert(ilist.begin(), ilist.end());
    }


    template<typename K, typename T, typename C, typename RAC>
    template<typename InputIterator>
    inline vector_map<K, T, C, RAC>::vector_map(InputIterator first, InputIterator last)
            : _cmp(), _data() {
        insert(first, last);
    }


    template<typename K, typename T, typename C, typename RAC>
    template<typename InputIterator>
    inline vector_map<K, T, C, RAC>::vector_map(InputIterator first, InputIterator last, const key_compare &compare)
            : value_compare(compare), _data() {
        insert(first, last);
    }


    template<typename K, typename T, typename C, typename RAC>
    inline vector_map<K, T, C, RAC> &
    vector_map<K, T, C, RAC>::operator=(const this_type &x) {
        _data = x._data;
        _cmp = x._cmp;
        return *this;
    }


    template<typename K, typename T, typename C, typename RAC>
    inline vector_map<K, T, C, RAC> &
    vector_map<K, T, C, RAC>::operator=(this_type &&x) {
        _data = std::move(x._data);
        std::swap(_cmp, x._cmp);
        return *this;
    }


    template<typename K, typename T, typename C, typename RAC>
    inline vector_map<K, T, C, RAC> &
    vector_map<K, T, C, RAC>::operator=(std::initializer_list<value_type> ilist) {
        _data.clear();
        insert(ilist.begin(), ilist.end());
        return *this;
    }


    template<typename K, typename T, typename C, typename RAC>
    inline void vector_map<K, T, C, RAC>::swap(this_type &x) {
        std::swap(_data, x._data);
        std::swap(_cmp, x._cmp);
    }


    template<typename K, typename T, typename C, typename RAC>
    inline const typename vector_map<K, T, C, RAC>::key_compare &
    vector_map<K, T, C, RAC>::key_comp() const {
        return static_cast<const key_compare &>(*this);
    }


    template<typename K, typename T, typename C, typename RAC>
    inline typename vector_map<K, T, C, RAC>::key_compare &
    vector_map<K, T, C, RAC>::key_comp() {
        return static_cast<key_compare &>(*this);
    }


    template<typename K, typename T, typename C, typename RAC>
    inline const typename vector_map<K, T, C, RAC>::value_compare &
    vector_map<K, T, C, RAC>::value_comp() const {
        return static_cast<const value_compare &>(*this);
    }


    template<typename K, typename T, typename C, typename RAC>
    inline typename vector_map<K, T, C, RAC>::value_compare &
    vector_map<K, T, C, RAC>::value_comp() {
        return static_cast<value_compare &>(*this);
    }


    template<typename K, typename T, typename C, typename RAC>
    template<class... Args>
    inline std::pair<typename vector_map<K, T, C, RAC>::iterator, bool>
    vector_map<K, T, C, RAC>::emplace(Args &&... args) {
#if EASTL_USE_FORWARD_WORKAROUND
        auto value = value_type(std::forward<Args>(args)...);  // Workaround for compiler bug in VS2013 which results in a compiler internal crash while compiling this code.
#else
        value_type value(std::forward<Args>(args)...);
#endif
        return insert(std::move(value));
    }


    template<typename K, typename T, typename C, typename RAC>
    template<class... Args>
    inline typename vector_map<K, T, C, RAC>::iterator
    vector_map<K, T, C, RAC>::emplace_hint(const_iterator position, Args &&... args) {
#if EASTL_USE_FORWARD_WORKAROUND
        auto value = value_type(std::forward<Args>(args)...);  // Workaround for compiler bug in VS2013 which results in a compiler internal crash while compiling this code.
#else
        value_type value(std::forward<Args>(args)...);
#endif

        return insert(position, std::move(value));
    }


    template<typename K, typename T, typename C, typename RAC>
    inline std::pair<typename vector_map<K, T, C, RAC>::iterator, bool>
    vector_map<K, T, C, RAC>::insert(const value_type &value) {
        const iterator itLB(lower_bound(value.first));

        if ((itLB != end()) && !value_compare::operator()(value, *itLB))
            return std::pair<iterator, bool>(itLB, false);

        return std::pair<iterator, bool>(base_type::insert(itLB, value), true);
    }


    template<typename K, typename T, typename C, typename RAC>
    template<typename P, typename>
    inline std::pair<typename vector_map<K, T, C, RAC>::iterator, bool>
    vector_map<K, T, C, RAC>::insert(P &&otherValue) {
        value_type value(std::forward<P>(otherValue));
        const iterator itLB(lower_bound(value.first));

        if ((itLB != end()) && !value_compare::operator()(value, *itLB))
            return std::pair<iterator, bool>(itLB, false);

        return std::pair<iterator, bool>(base_type::insert(itLB, std::move(value)), true);
    }


    template<typename K, typename T, typename C, typename RAC>
    inline std::pair<typename vector_map<K, T, C, RAC>::iterator, bool>
    vector_map<K, T, C, RAC>::insert(const key_type &otherValue) {
        value_type value(otherValue);
        const iterator itLB(lower_bound(value.first));

        if ((itLB != end()) && !value_compare::operator()(value, *itLB))
            return std::pair<iterator, bool>(itLB, false);

        return std::pair<iterator, bool>(base_type::insert(itLB, std::move(value)), true);
    }

    template<typename K, typename T, typename C, typename RAC>
    inline std::pair<typename vector_map<K, T, C, RAC>::iterator, bool>
    vector_map<K, T, C, RAC>::insert(key_type &&otherValue) {
        value_type value(std::move(otherValue));
        const iterator itLB(lower_bound(value.first));

        if ((itLB != end()) && !value_compare::operator()(value, *itLB))
            return std::pair<iterator, bool>(itLB, false);

        return std::pair<iterator, bool>(base_type::insert(itLB, std::move(value)), true);
    }


    template<typename K, typename T, typename C, typename RAC>
    typename vector_map<K, T, C, RAC>::iterator
    vector_map<K, T, C, RAC>::insert(const_iterator position, const value_type &value) {
        // We assume that the user knows what he is doing and has supplied us with
        // a position that is right where value should be inserted (put in front of).
        // We do a test to see if the position is correct. If so then we insert,
        // if not then we ignore the input position.

        if ((position == end()) ||
            value_compare::operator()(value, *position))  // If the element at position is greater than value...
        {
            if ((position == begin()) || value_compare::operator()(*(position - 1),
                                                                   value)) // If the element before position is less than value...
                return base_type::insert(position, value);
        }

        // In this case we either have an incorrect position or value is already present.
        // We fall back to the regular insert function. An optimization would be to detect
        // that the element is already present, but that's only useful if the user supplied
        // a good position but a present element.
        const std::pair<typename vector_map<K, T, C, RAC>::iterator, bool> result = insert(value);

        return result.first;
    }


    template<typename K, typename T, typename C, typename RAC>
    typename vector_map<K, T, C, RAC>::iterator
    vector_map<K, T, C, RAC>::insert(const_iterator position, value_type &&value) {
        if ((position == end()) ||
            value_compare::operator()(value, *position))  // If the element at position is greater than value...
        {
            if ((position == begin()) || value_compare::operator()(*(position - 1),
                                                                   value)) // If the element before position is less than value...
                return base_type::insert(position, std::move(value));
        }

        const std::pair<typename vector_map<K, T, C, RAC>::iterator, bool> result = insert(std::move(value));

        return result.first;
    }


    template<typename K, typename T, typename C, typename RAC>
    inline void vector_map<K, T, C, RAC>::insert(std::initializer_list<value_type> ilist) {
        insert(ilist.begin(), ilist.end());
    }


    template<typename K, typename T, typename C, typename RAC>
    template<typename InputIterator>
    inline void vector_map<K, T, C, RAC>::insert(InputIterator first, InputIterator last) {
        // To consider: Improve the speed of this by getting the length of the
        //              input range and resizing our container to that size
        //              before doing the insertions. We can't use reserve
        //              because we don't know if we are using a vector or not.
        //              Alternatively, force the user to do the reservation.
        // To consider: When inserting values that come from a container
        //              like this container, use the property that they are
        //              known to be sorted and speed up the inserts here.
        for (; first != last; ++first)
            insert(*first);
    }


    template<typename K, typename T, typename C, typename RAC>
    inline typename vector_map<K, T, C, RAC>::iterator
    vector_map<K, T, C, RAC>::erase(const_iterator position) {
        // Note that we return iterator and not void. This allows for more efficient use of
        // the container and is consistent with the C++ language defect report #130 (DR 130)
        return base_type::erase(position);
    }


    template<typename K, typename T, typename C, typename RAC>
    inline typename vector_map<K, T, C, RAC>::iterator
    vector_map<K, T, C, RAC>::erase(const_iterator first, const_iterator last) {
        return base_type::erase(first, last);
    }


    template<typename K, typename T, typename C, typename RAC>
    inline typename vector_map<K, T, C, RAC>::size_type
    vector_map<K, T, C, RAC>::erase(const key_type &k) {
        const iterator it(find(k));

        if (it != end()) // If it exists...
        {
            erase(it);
            return 1;
        }
        return 0;
    }


    template<typename K, typename T, typename C, typename RAC>
    inline typename vector_map<K, T, C, RAC>::reverse_iterator
    vector_map<K, T, C, RAC>::erase(const_reverse_iterator position) {
        return reverse_iterator(base_type::erase((++position).base()));
    }


    template<typename K, typename T, typename C, typename RAC>
    inline typename vector_map<K, T, C, RAC>::reverse_iterator
    vector_map<K, T, C, RAC>::erase(const_reverse_iterator first, const_reverse_iterator last) {
        return reverse_iterator(base_type::erase((++last).base(), (++first).base()));
    }


    template<typename K, typename T, typename C, typename RAC>
    inline typename vector_map<K, T, C, RAC>::iterator
    vector_map<K, T, C, RAC>::find(const key_type &k) {
        const std::pair<iterator, iterator> pairIts(equal_range(k));
        return (pairIts.first != pairIts.second) ? pairIts.first : end();
    }


    template<typename K, typename T, typename C, typename RAC>
    inline typename vector_map<K, T, C, RAC>::const_iterator
    vector_map<K, T, C, RAC>::find(const key_type &k) const {
        const std::pair<const_iterator, const_iterator> pairIts(equal_range(k));
        return (pairIts.first != pairIts.second) ? pairIts.first : end();
    }


    template<typename K, typename T, typename C, typename RAC>
    template<typename U, typename BinaryPredicate>
    inline typename vector_map<K, T, C, RAC>::iterator
    vector_map<K, T, C, RAC>::find_as(const U &u, BinaryPredicate predicate) {
        const std::pair<iterator, iterator> pairIts(equal_range(u, predicate));
        return (pairIts.first != pairIts.second) ? pairIts.first : end();
    }


    template<typename K, typename T, typename C, typename RAC>
    template<typename U, typename BinaryPredicate>
    inline typename vector_map<K, T, C, RAC>::const_iterator
    vector_map<K, T, C, RAC>::find_as(const U &u, BinaryPredicate predicate) const {
        const std::pair<const_iterator, const_iterator> pairIts(equal_range(u, predicate));
        return (pairIts.first != pairIts.second) ? pairIts.first : end();
    }


    template<typename K, typename T, typename C, typename RAC>
    inline typename vector_map<K, T, C, RAC>::size_type
    vector_map<K, T, C, RAC>::count(const key_type &k) const {
        const const_iterator it(find(k));
        return (it != end()) ? (size_type) 1 : (size_type) 0;
    }


    template<typename K, typename T, typename C, typename RAC>
    inline typename vector_map<K, T, C, RAC>::iterator
    vector_map<K, T, C, RAC>::lower_bound(const key_type &k) {
        return std::lower_bound(begin(), end(), k, static_cast<value_compare &>(*this));
    }


    template<typename K, typename T, typename C, typename RAC>
    inline typename vector_map<K, T, C, RAC>::const_iterator
    vector_map<K, T, C, RAC>::lower_bound(const key_type &k) const {
        return std::lower_bound(begin(), end(), k, static_cast<const value_compare &>(*this));
    }


    template<typename K, typename T, typename C, typename RAC>
    inline typename vector_map<K, T, C, RAC>::iterator
    vector_map<K, T, C, RAC>::upper_bound(const key_type &k) {
        return std::upper_bound(begin(), end(), k, static_cast<value_compare &>(*this));
    }


    template<typename K, typename T, typename C, typename RAC>
    inline typename vector_map<K, T, C, RAC>::const_iterator
    vector_map<K, T, C, RAC>::upper_bound(const key_type &k) const {
        return std::upper_bound(begin(), end(), k, static_cast<const value_compare &>(*this));
    }


    template<typename K, typename T, typename C, typename RAC>
    inline std::pair<typename vector_map<K, T, C, RAC>::iterator, typename vector_map<K, T, C, RAC>::iterator>
    vector_map<K, T, C, RAC>::equal_range(const key_type &k) {
        // The resulting range will either be empty or have one element,
        // so instead of doing two tree searches (one for lower_bound and
        // one for upper_bound), we do just lower_bound and see if the
        // result is a range of size zero or one.
        const iterator itLower(lower_bound(k));

        if ((itLower == end()) || value_compare::operator()(k, *itLower)) // If at the end or if (k is < itLower)...
            return std::pair<iterator, iterator>(itLower, itLower);

        iterator itUpper(itLower);
        return std::pair<iterator, iterator>(itLower, ++itUpper);
    }


    template<typename K, typename T, typename C, typename RAC>
    inline std::pair<typename vector_map<K, T, C, RAC>::const_iterator, typename vector_map<K, T, C, RAC>::const_iterator>
    vector_map<K, T, C, RAC>::equal_range(const key_type &k) const {
        // The resulting range will either be empty or have one element,
        // so instead of doing two tree searches (one for lower_bound and
        // one for upper_bound), we do just lower_bound and see if the
        // result is a range of size zero or one.
        const const_iterator itLower(lower_bound(k));

        if ((itLower == end()) || value_compare::operator()(k, *itLower)) // If at the end or if (k is < itLower)...
            return std::pair<const_iterator, const_iterator>(itLower, itLower);

        const_iterator itUpper(itLower);
        return std::pair<const_iterator, const_iterator>(itLower, ++itUpper);
    }

    template<typename K, typename T, typename C, typename RAC>
    template<typename U, typename BinaryPredicate>
    inline std::pair<typename vector_map<K, T, C, RAC>::iterator, typename vector_map<K, T, C, RAC>::iterator>
    vector_map<K, T, C, RAC>::equal_range(const U &u, BinaryPredicate predicate) {
        // The resulting range will either be empty or have one element,
        // so instead of doing two tree searches (one for lower_bound and
        // one for upper_bound), we do just lower_bound and see if the
        // result is a range of size zero or one.
        map_value_compare<U, value_type, BinaryPredicate> predicate_cmp(predicate);

        const iterator itLower(std::lower_bound(begin(), end(), u, predicate_cmp));

        if ((itLower == end()) || predicate_cmp(u, *itLower)) // If at the end or if (k is < itLower)...
            return std::pair<iterator, iterator>(itLower, itLower);

        iterator itUpper(itLower);
        return std::pair<iterator, iterator>(itLower, ++itUpper);
    }


    template<typename K, typename T, typename C, typename RAC>
    template<typename U, typename BinaryPredicate>
    inline std::pair<typename vector_map<K, T, C, RAC>::const_iterator, typename vector_map<K, T, C, RAC>::const_iterator>
    vector_map<K, T, C, RAC>::equal_range(const U &u, BinaryPredicate predicate) const {
        // The resulting range will either be empty or have one element,
        // so instead of doing two tree searches (one for lower_bound and
        // one for upper_bound), we do just lower_bound and see if the
        // result is a range of size zero or one.
        map_value_compare<U, value_type, BinaryPredicate> predicate_cmp(predicate);

        const const_iterator itLower(std::lower_bound(begin(), end(), u, predicate_cmp));

        if ((itLower == end()) || predicate_cmp(u, *itLower)) // If at the end or if (k is < itLower)...
            return std::pair<const_iterator, const_iterator>(itLower, itLower);

        const_iterator itUpper(itLower);
        return std::pair<const_iterator, const_iterator>(itLower, ++itUpper);
    }


    template<typename K, typename T, typename C, typename RAC>
    inline typename vector_map<K, T, C, RAC>::mapped_type &
    vector_map<K, T, C, RAC>::operator[](const key_type &k) {
        iterator itLB(lower_bound(k));

        if ((itLB == end()) || key_comp()(k, (*itLB).first))
            itLB = insert(itLB, value_type(k, mapped_type()));
        return (*itLB).second;
    }


    template<typename K, typename T, typename C, typename RAC>
    inline typename vector_map<K, T, C, RAC>::mapped_type &
    vector_map<K, T, C, RAC>::operator[](key_type &&k) {
        iterator itLB(lower_bound(k));

        if ((itLB == end()) || key_comp()(k, (*itLB).first))
            itLB = insert(itLB, value_type(std::move(k), mapped_type()));
        return (*itLB).second;
    }

    template<typename K, typename T, typename C, typename RAC>
    inline typename vector_map<K, T, C, RAC>::reference
    vector_map<K, T, C, RAC>::at(size_type index) {
        return *(begin() + index);
    }

    template<typename K, typename T, typename C, typename RAC>
    inline typename vector_map<K, T, C, RAC>::const_reference
    vector_map<K, T, C, RAC>::at(size_type index) const {
        return *(begin() + index);
    }

    template<typename K, typename T, typename C, typename RAC>
    inline typename vector_map<K, T, C, RAC>::mapped_type &
    vector_map<K, T, C, RAC>::at_key(const key_type &k) {
        // use the use const version of ::at to remove duplication
        return const_cast<mapped_type &>(const_cast<vector_map<K, T, C, RAC> const *>(this)->at_key(k));
    }

    template<typename K, typename T, typename C, typename RAC>
    inline const typename vector_map<K, T, C, RAC>::mapped_type &
    vector_map<K, T, C, RAC>::at_key(const key_type &k) const {
        const_iterator itLB(lower_bound(k));

        if ((itLB == end()) || key_comp()(k, itLB->first)) {
            throw std::out_of_range("vector_map::at key does not exist");
        }

        return itLB->second;
    }



    ///////////////////////////////////////////////////////////////////////////
    // global operators
    ///////////////////////////////////////////////////////////////////////////

    template<typename Key, typename T, typename Compare, typename RandomAccessContainer>
    inline bool operator==(const vector_map<Key, T, Compare, RandomAccessContainer> &a,
                           const vector_map<Key, T, Compare, RandomAccessContainer> &b) {
        return (a.size() == b.size()) && std::equal(b.begin(), b.end(), a.begin());
    }


    template<typename Key, typename T, typename Compare, typename RandomAccessContainer>
    inline bool operator<(const vector_map<Key, T, Compare, RandomAccessContainer> &a,
                          const vector_map<Key, T, Compare, RandomAccessContainer> &b) {
        return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), a.value_comp());
    }


    template<typename Key, typename T, typename Compare, typename RandomAccessContainer>
    inline bool operator!=(const vector_map<Key, T, Compare, RandomAccessContainer> &a,
                           const vector_map<Key, T, Compare, RandomAccessContainer> &b) {
        return !(a == b);
    }


    template<typename Key, typename T, typename Compare, typename RandomAccessContainer>
    inline bool operator>(const vector_map<Key, T, Compare, RandomAccessContainer> &a,
                          const vector_map<Key, T, Compare, RandomAccessContainer> &b) {
        return b < a;
    }


    template<typename Key, typename T, typename Compare, typename RandomAccessContainer>
    inline bool operator<=(const vector_map<Key, T, Compare, RandomAccessContainer> &a,
                           const vector_map<Key, T, Compare, RandomAccessContainer> &b) {
        return !(b < a);
    }


    template<typename Key, typename T, typename Compare, typename RandomAccessContainer>
    inline bool operator>=(const vector_map<Key, T, Compare, RandomAccessContainer> &a,
                           const vector_map<Key, T, Compare, RandomAccessContainer> &b) {
        return !(a < b);
    }


    template<typename Key, typename T, typename Compare, typename RandomAccessContainer>
    inline void swap(vector_map<Key, T, Compare, RandomAccessContainer> &a,
                     vector_map<Key, T, Compare, RandomAccessContainer> &b) {
        a.swap(b);
    }


} // namespace turbo

















