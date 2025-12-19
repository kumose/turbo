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

#include <cassert>
#include <functional>
#include <memory>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <turbo/meta/compare.h>
#include <turbo/functional/functional.h>
#include <turbo/base/macros.h>

namespace turbo {

    template<typename T>
    class Iterator;

    template<typename T>
    struct IterationTraits {
        /// \brief a reserved value which indicates the end of iteration. By
        /// default this is nullptr since most iterators yield pointer types.
        /// Specialize IterationTraits if different end semantics are required.
        ///
        /// Note: This should not be used to determine if a given value is a
        /// terminal value.  Use is_iteration_end (which uses is_end) instead.  This
        /// is only for returning terminal values.
        static T end() { return T(nullptr); }

        /// \brief Checks to see if the value is a terminal value.
        /// A method is used here since T is not necessarily comparable in many
        /// cases even though it has a distinct final value
        static bool is_end(const T &val) { return val == end(); }
    };

    template<typename T>
    T iteration_end() {
        return IterationTraits<T>::end();
    }

    template<typename T>
    bool is_iteration_end(const T &val) {
        return IterationTraits<T>::is_end(val);
    }

    template<typename T>
    struct IterationTraits<std::optional<T>> {
        /// \brief by default when iterating through a sequence of optional,
        /// nullopt indicates the end of iteration.
        /// Specialize IterationTraits if different end semantics are required.
        static std::optional<T> end() { return std::nullopt; }

        /// \brief by default when iterating through a sequence of optional,
        /// nullopt (!has_value()) indicates the end of iteration.
        /// Specialize IterationTraits if different end semantics are required.
        static bool is_end(const std::optional<T> &val) { return !val.has_value(); }

        // TODO(bkietz) The range-for loop over Iterator<optional<T>> yields
        // turbo::Result<optional<T>> which is unnecessary (since only the unyielded end optional
        // is nullopt. Add IterationTraits::GetRangeElement() to handle this case
    };

    /// \brief A generic Iterator that can return errors
    template<typename T>
    class Iterator : public turbo::EqualityComparable<Iterator<T>> {
    public:
        /// \brief Iterator may be constructed from any type which has a member function
        /// with signature turbo::Result<T> next();
        /// end of iterator is signalled by returning IteratorTraits<T>::end();
        ///
        /// The argument is moved or copied to the heap and kept in a unique_ptr<void>. Only
        /// its destructor and its next method (which are stored in function pointers) are
        /// referenced after construction.
        ///
        /// This approach is used to dodge MSVC linkage hell when using
        /// an abstract template base class: instead of being inlined as usual for a template
        /// function the base's virtual destructor will be exported, leading to multiple
        /// definition errors when linking to any other TU where the base is instantiated.
        template<typename Wrapped>
        explicit Iterator(Wrapped has_next)
                : ptr_(new Wrapped(std::move(has_next)), remove < Wrapped > ), next_(next < Wrapped > ) {}

        Iterator() : ptr_(nullptr, [](void *) {}) {}

        /// \brief Return the next element of the sequence, IterationTraits<T>::end() when the
        /// iteration is completed.
        turbo::Result<T> next() {
            if (ptr_) {
                auto next_result = next_(ptr_.get());
                if (next_result.ok() && is_iteration_end(next_result.value_unsafe())) {
                    ptr_.reset(nullptr);
                }
                return next_result;
            } else {
                return IterationTraits<T>::end();
            }
        }

        /// Pass each element of the sequence to a visitor. Will return any error status
        /// returned by the visitor, terminating iteration.
        template<typename Visitor>
        turbo::Status Visit(Visitor &&visitor) {
            for (;;) {
                TURBO_MOVE_OR_RAISE(auto value, next());

                if (is_iteration_end(value)) break;

                TURBO_RETURN_NOT_OK(visitor(std::move(value)));
            }

            return turbo::OkStatus();
        }

        /// Iterators will only compare equal if they are both null.
        /// Equality comparability is required to make an Iterator of Iterators
        /// (to check for the end condition).
        bool equals(const Iterator &other) const { return ptr_ == other.ptr_; }

        explicit operator bool() const { return ptr_ != nullptr; }

        class RangeIterator {
        public:
            RangeIterator() : value_(IterationTraits<T>::end()) {}

            explicit RangeIterator(Iterator i)
                    : value_(IterationTraits<T>::end()),
                      iterator_(std::make_shared<Iterator>(std::move(i))) {
                next();
            }

            bool operator!=(const RangeIterator &other) const { return value_ != other.value_; }

            RangeIterator &operator++() {
                next();
                return *this;
            }

            turbo::Result<T> operator*() {
                TURBO_RETURN_NOT_OK(value_.status());

                auto value = std::move(value_);
                value_ = IterationTraits<T>::end();
                return value;
            }

        private:
            void next() {
                if (!value_.ok()) {
                    value_ = IterationTraits<T>::end();
                    return;
                }
                value_ = iterator_->next();
            }

            turbo::Result<T> value_;
            std::shared_ptr<Iterator> iterator_;
        };

        RangeIterator begin() { return RangeIterator(std::move(*this)); }

        RangeIterator end() { return RangeIterator(); }

        /// \brief Move every element of this iterator into a vector.
        turbo::Result<std::vector<T>> to_vector() {
            std::vector<T> out;
            for (auto maybe_element: *this) {
                TURBO_MOVE_OR_RAISE(auto element, maybe_element);
                out.push_back(std::move(element));
            }
            return out;
        }

    private:
        /// Implementation of deleter for ptr_: Casts from void* to the wrapped type and
        /// deletes that.
        template<typename HasNext>
        static void remove(void *ptr) {
            delete static_cast<HasNext *>(ptr);
        }

        /// Implementation of next: Casts from void* to the wrapped type and invokes that
        /// type's next member function.
        template<typename HasNext>
        static turbo::Result<T> next(void *ptr) {
            return static_cast<HasNext *>(ptr)->next();
        }

        /// ptr_ is a unique_ptr to void with a custom deleter: a function pointer which first
        /// casts from void* to a pointer to the wrapped type then deletes that.
        std::unique_ptr<void, void (*)(void *)> ptr_;

        /// next_ is a function pointer which first casts from void* to a pointer to the wrapped
        /// type then invokes its next member function.
        turbo::Result<T> (*next_)(void *) = nullptr;
    };

    template<typename T>
    struct TransformFlow {
        using YieldValueType = T;

        TransformFlow(YieldValueType value, bool ready_for_next)
                : finished_(false),
                  ready_for_next_(ready_for_next),
                  yield_value_(std::move(value)) {}

        TransformFlow(bool finished, bool ready_for_next)
                : finished_(finished), ready_for_next_(ready_for_next), yield_value_() {}

        bool has_value() const { return yield_value_.has_value(); }

        bool finished() const { return finished_; }

        bool ready_for_next() const { return ready_for_next_; }

        T value() const { return *yield_value_; }

        bool finished_ = false;
        bool ready_for_next_ = false;
        std::optional<YieldValueType> yield_value_;
    };

    struct TransformFinish {
        template<typename T>
        operator TransformFlow<T>() &&{  // NOLINT explicit
            return TransformFlow<T>(true, true);
        }
    };

    struct TransformSkip {
        template<typename T>
        operator TransformFlow<T>() &&{  // NOLINT explicit
            return TransformFlow<T>(false, true);
        }
    };

    template<typename T>
    TransformFlow<T> TransformYield(T value = {}, bool ready_for_next = true) {
        return TransformFlow<T>(std::move(value), ready_for_next);
    }

    template<typename T, typename V>
    using Transformer = std::function<turbo::Result<TransformFlow<V>>(T)>;

    template<typename T, typename V>
    class TransformIterator {
    public:
        explicit TransformIterator(Iterator<T> it, Transformer<T, V> transformer)
                : it_(std::move(it)),
                  transformer_(std::move(transformer)),
                  last_value_(),
                  finished_() {}

        turbo::Result<V> next() {
            while (!finished_) {
                TURBO_MOVE_OR_RAISE(std::optional<V> next, Pump());
                if (next.has_value()) {
                    return std::move(*next);
                }
                TURBO_MOVE_OR_RAISE(last_value_, it_.next());
            }
            return IterationTraits<V>::end();
        }

    private:
        // Calls the transform function on the current value.  Can return in several ways
        // * If the next value is requested (e.g. skip) it will return an empty optional
        // * If an invalid status is encountered that will be returned
        // * If finished it will return IterationTraits<V>::end()
        // * If a value is returned by the transformer that will be returned
        turbo::Result<std::optional<V>> Pump() {
            if (!finished_ && last_value_.has_value()) {
                auto next_res = transformer_(*last_value_);
                if (!next_res.ok()) {
                    finished_ = true;
                    return next_res.status();
                }
                auto next = *next_res;
                if (next.ready_for_next()) {
                    if (is_iteration_end(*last_value_)) {
                        finished_ = true;
                    }
                    last_value_.reset();
                }
                if (next.finished()) {
                    finished_ = true;
                }
                if (next.has_value()) {
                    return next.value();
                }
            }
            if (finished_) {
                return IterationTraits<V>::end();
            }
            return std::nullopt;
        }

        Iterator<T> it_;
        Transformer<T, V> transformer_;
        std::optional<T> last_value_;
        bool finished_ = false;
    };

    /// \brief Transforms an iterator according to a transformer, returning a new Iterator.
    ///
    /// The transformer will be called on each element of the source iterator and for each
    /// call it can yield a value, skip, or finish the iteration.  When yielding a value the
    /// transformer can choose to consume the source item (the default, ready_for_next = true)
    /// or to keep it and it will be called again on the same value.
    ///
    /// This is essentially a more generic form of the map operation that can return 0, 1, or
    /// many values for each of the source items.
    ///
    /// The transformer will be exposed to the end of the source sequence
    /// (IterationTraits::end) in case it needs to return some penultimate item(s).
    ///
    /// Any invalid status returned by the transformer will be returned immediately.
    template<typename T, typename V>
    Iterator<V> make_transformed_iterator(Iterator<T> it, Transformer<T, V> op) {
        return Iterator<V>(TransformIterator<T, V>(std::move(it), std::move(op)));
    }

    template<typename T>
    struct IterationTraits<Iterator<T>> {
        // The end condition for an Iterator of Iterators is a default constructed (null)
        // Iterator.
        static Iterator<T> end() { return Iterator<T>(); }

        static bool is_end(const Iterator<T> &val) { return !val; }
    };

    template<typename Fn, typename T>
    class FunctionIterator {
    public:
        explicit FunctionIterator(Fn fn) : fn_(std::move(fn)) {}

        turbo::Result<T> next() { return fn_(); }

    private:
        Fn fn_;
    };

    /// \brief Construct an Iterator which invokes a callable on next()
    template<typename Fn,
            typename Ret = typename call_traits::return_type<Fn>::value_type>
    Iterator<Ret> make_function_iterator(Fn fn) {
        return Iterator<Ret>(FunctionIterator<Fn, Ret>(std::move(fn)));
    }

    template<typename T>
    Iterator<T> make_empty_iterator() {
        return make_function_iterator([]() -> turbo::Result<T> { return IterationTraits<T>::end(); });
    }

    template<typename T>
    Iterator<T> make_error_iterator(turbo::Status s) {
        return make_function_iterator([s]() -> turbo::Result<T> {
            TURBO_RETURN_NOT_OK(s);
            return IterationTraits<T>::end();
        });
    }

    /// \brief Simple iterator which yields the elements of a std::vector
    template<typename T>
    class VectorIterator {
    public:
        explicit VectorIterator(std::vector<T> v) : elements_(std::move(v)) {}

        turbo::Result<T> next() {
            if (i_ == elements_.size()) {
                return IterationTraits<T>::end();
            }
            return std::move(elements_[i_++]);
        }

    private:
        std::vector<T> elements_;
        size_t i_ = 0;
    };

    template<typename T>
    Iterator<T> make_vector_iterator(std::vector<T> v) {
        return Iterator<T>(VectorIterator<T>(std::move(v)));
    }

    /// \brief Simple iterator which yields *pointers* to the elements of a std::vector<T>.
    /// This is provided to support T where IterationTraits<T>::end is not specialized
    template<typename T>
    class VectorPointingIterator {
    public:
        explicit VectorPointingIterator(std::vector<T> v) : elements_(std::move(v)) {}

        turbo::Result<T *> next() {
            if (i_ == elements_.size()) {
                return nullptr;
            }
            return &elements_[i_++];
        }

    private:
        std::vector<T> elements_;
        size_t i_ = 0;
    };

    template<typename T>
    Iterator<T *> make_vector_pointing_iterator(std::vector<T> v) {
        return Iterator<T *>(VectorPointingIterator<T>(std::move(v)));
    }

    /// \brief MapIterator takes ownership of an iterator and a function to apply
    /// on every element. The mapped function is not allowed to fail.
    template<typename Fn, typename I, typename O>
    class MapIterator {
    public:
        explicit MapIterator(Fn map, Iterator<I> it)
                : map_(std::move(map)), it_(std::move(it)) {}

        turbo::Result<O> next() {
            TURBO_MOVE_OR_RAISE(I i, it_.next());

            if (is_iteration_end(i)) {
                return IterationTraits<O>::end();
            }

            return map_(std::move(i));
        }

    private:
        Fn map_;
        Iterator<I> it_;
    };

    /// \brief MapIterator takes ownership of an iterator and a function to apply
    /// on every element. The mapped function is not allowed to fail.
    template<typename Fn, typename From = call_traits::argument_type<0, Fn>,
            typename To = call_traits::return_type<Fn>>
    Iterator<To> make_map_iterator(Fn map, Iterator<From> it) {
        return Iterator<To>(MapIterator<Fn, From, To>(std::move(map), std::move(it)));
    }

    /// \brief Like MapIterator, but where the function can fail.
    template<typename Fn, typename From = call_traits::argument_type<0, Fn>,
            typename To = typename call_traits::return_type<Fn>::value_type>
    Iterator<To> make_maybe_map_iterator(Fn map, Iterator<From> it) {
        return Iterator<To>(MapIterator<Fn, From, To>(std::move(map), std::move(it)));
    }

    struct FilterIterator {
        enum Action {
            ACCEPT, REJECT
        };

        template<typename To>
        static turbo::Result<std::pair<To, Action>> reject() {
            return std::make_pair(IterationTraits<To>::end(), REJECT);
        }

        template<typename To>
        static turbo::Result<std::pair<To, Action>> accept(To out) {
            return std::make_pair(std::move(out), ACCEPT);
        }

        template<typename To>
        static turbo::Result<std::pair<To, Action>> maybe_accept(turbo::Result<To> maybe_out) {
            return std::move(maybe_out).mapping(accept<To>);
        }

        template<typename To>
        static turbo::Result<std::pair<To, Action>> error(turbo::Status s) {
            return s;
        }

        template<typename Fn, typename From, typename To>
        class Impl {
        public:
            explicit Impl(Fn filter, Iterator<From> it) : filter_(filter), it_(std::move(it)) {}

            turbo::Result<To> next() {
                To out = IterationTraits<To>::end();
                Action action;

                for (;;) {
                    TURBO_MOVE_OR_RAISE(From i, it_.next());

                    if (is_iteration_end(i)) {
                        return IterationTraits<To>::end();
                    }

                    TURBO_MOVE_OR_RAISE(std::tie(out, action), filter_(std::move(i)));

                    if (action == ACCEPT) return out;
                }
            }

        private:
            Fn filter_;
            Iterator<From> it_;
        };
    };

    /// \brief Like MapIterator, but where the function can fail or reject elements.
    template<
            typename Fn, typename From = typename call_traits::argument_type<0, Fn>,
            typename Ret = typename call_traits::return_type<Fn>::value_type,
            typename To = typename std::tuple_element<0, Ret>::type,
            typename Enable = typename std::enable_if<std::is_same<
                    typename std::tuple_element<1, Ret>::type, FilterIterator::Action>::value>::type>
    Iterator<To> make_filter_iterator(Fn filter, Iterator<From> it) {
        return Iterator<To>(
                FilterIterator::Impl<Fn, From, To>(std::move(filter), std::move(it)));
    }

    /// \brief FlattenIterator takes an iterator generating iterators and yields a
    /// unified iterator that flattens/concatenates in a single stream.
    template<typename T>
    class FlattenIterator {
    public:
        explicit FlattenIterator(Iterator<Iterator<T>> it) : parent_(std::move(it)) {}

        turbo::Result<T> next() {
            if (is_iteration_end(child_)) {
                // Pop from parent's iterator.
                TURBO_MOVE_OR_RAISE(child_, parent_.next());

                // Check if final iteration reached.
                if (is_iteration_end(child_)) {
                    return IterationTraits<T>::end();
                }

                return next();
            }

            // Pop from child_ and check for depletion.
            TURBO_MOVE_OR_RAISE(T out, child_.next());
            if (is_iteration_end(out)) {
                // Reset state such that we pop from parent on the recursive call
                child_ = IterationTraits<Iterator<T>>::end();

                return next();
            }

            return out;
        }

    private:
        Iterator<Iterator<T>> parent_;
        Iterator<T> child_ = IterationTraits<Iterator<T>>::end();
    };

    template<typename T>
    Iterator<T> make_flatten_iterator(Iterator<Iterator<T>> it) {
        return Iterator<T>(FlattenIterator<T>(std::move(it)));
    }

    template<typename Reader>
    Iterator<typename Reader::value_type> make_iterator_from_reader(
            const std::shared_ptr<Reader> &reader) {
        return make_function_iterator([reader] { return reader->next(); });
    }

}  // namespace turbo
