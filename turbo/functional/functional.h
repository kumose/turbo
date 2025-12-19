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
#include <tuple>
#include <type_traits>

#include <turbo/utility/status.h>
#include <turbo/base/macros.h>

namespace turbo {
    /// Helper struct for examining lambdas and other callables.
    struct call_traits {
    public:
        template<typename R, typename... A>
        static std::false_type is_overloaded_impl(R(A...));

        template<typename F>
        static std::false_type is_overloaded_impl(decltype(&F::operator()) *);

        template<typename F>
        static std::true_type is_overloaded_impl(...);

        template<typename F, typename R, typename... A>
        static R return_type_impl(R (F::*)(A...));

        template<typename F, typename R, typename... A>
        static R return_type_impl(R (F::*)(A...) const);

        template<std::size_t I, typename F, typename R, typename... A>
        static typename std::tuple_element<I, std::tuple<A...>>::type argument_type_impl(
                R (F::*)(A...));

        template<std::size_t I, typename F, typename R, typename... A>
        static typename std::tuple_element<I, std::tuple<A...>>::type argument_type_impl(
                R (F::*)(A...) const);

        template<std::size_t I, typename F, typename R, typename... A>
        static typename std::tuple_element<I, std::tuple<A...>>::type argument_type_impl(
                R (F::*)(A...) &&);

        template<typename F, typename R, typename... A>
        static std::integral_constant<int, sizeof...(A)> argument_count_impl(R (F::*)(A...));

        template<typename F, typename R, typename... A>
        static std::integral_constant<int, sizeof...(A)> argument_count_impl(R (F::*)(A...)
        const);

        template<typename F, typename R, typename... A>
        static std::integral_constant<int, sizeof...(A)> argument_count_impl(R (F::*)(A...) &&);

        /// bool constant indicating whether F is a callable with more than one possible
        /// signature. Will be true_type for objects which define multiple operator() or which
        /// define a template operator()
        template<typename F>
        using is_overloaded =
                decltype(is_overloaded_impl<typename std::decay<F>::type>(nullptr));

        template<typename F, typename T = void>
        using enable_if_overloaded = typename std::enable_if<is_overloaded<F>::value, T>::type;

        template<typename F, typename T = void>
        using disable_if_overloaded =
                typename std::enable_if<!is_overloaded<F>::value, T>::type;

        /// If F is not overloaded, the argument types of its call operator can be
        /// extracted via call_traits::argument_type<Index, F>
        template<std::size_t I, typename F>
        using argument_type = decltype(argument_type_impl<I>(&std::decay<F>::type::operator()));

        template<typename F>
        using argument_count = decltype(argument_count_impl(&std::decay<F>::type::operator()));

        template<typename F>
        using return_type = decltype(return_type_impl(&std::decay<F>::type::operator()));

        template<typename F, typename T, typename RT = T>
        using enable_if_return =
                typename std::enable_if<std::is_same<return_type<F>, T>::value, RT>;

        template<typename T, typename R = void>
        using enable_if_empty = typename std::enable_if<std::is_same<T, EmptyResult>::value, R>::type;

        template<typename T, typename R = void>
        using enable_if_not_empty =
                typename std::enable_if<!std::is_same<T, EmptyResult>::value, R>::type;
    };

    /// A type erased callable object which may only be invoked once.
    /// It can be constructed from any lambda which matches the provided call signature.
    /// Invoking it results in destruction of the lambda, freeing any state/references
    /// immediately. Invoking a default constructed FnOnce or one which has already been
    /// invoked will segfault.
    template<typename Signature>
    class FnOnce;

    template<typename R, typename... A>
    class FnOnce<R(A...)> {
    public:
        FnOnce() = default;

        template<typename Fn,
                typename = typename std::enable_if<std::is_convertible<
                        decltype(std::declval<Fn &&>()(std::declval<A>()...)), R>::value>::type>
        FnOnce(Fn fn) : impl_(new FnImpl<Fn>(std::move(fn))) {  // NOLINT runtime/explicit
        }

        explicit operator bool() const { return impl_ != nullptr; }

        R operator()(A... a) &&{
            auto bye = std::move(impl_);
            return bye->invoke(std::forward<A &&>(a)...);
        }

    private:
        struct Impl {
            virtual ~Impl() = default;

            virtual R invoke(A &&... a) = 0;
        };

        template<typename Fn>
        struct FnImpl : Impl {
            explicit FnImpl(Fn fn) : fn_(std::move(fn)) {}

            R invoke(A &&... a) override { return std::move(fn_)(std::forward<A &&>(a)...); }

            Fn fn_;
        };

        std::unique_ptr<Impl> impl_;
    };

}  // namespace turbo

