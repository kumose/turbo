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

#pragma once

#include <stddef.h>  // For size_t.
#include <string.h>  // For memcpy.
#include <stdlib.h>
#include <turbo/base/macros/compiler_specific.h>  // For ALLOW_UNUSED.
#include <turbo/base/macros/build_config.h>

#define TURBO_DELETE_FUNCTION(decl) decl = delete

// Put this in the private: declarations for a class to be uncopyable.
#ifndef TURBO_DISALLOW_COPY
#define TURBO_DISALLOW_COPY(TypeName)                         \
    TURBO_DELETE_FUNCTION(TypeName(const TypeName&))
#endif

// Put this in the private: declarations for a class to be unassignable.
#ifndef TURBO_DISALLOW_ASSIGN
#define TURBO_DISALLOW_ASSIGN(TypeName)                               \
    TURBO_DELETE_FUNCTION(void operator=(const TypeName&))
#endif
// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#ifndef TURBO_DISALLOW_COPY_AND_ASSIGN
#define TURBO_DISALLOW_COPY_AND_ASSIGN(TypeName)                      \
    TURBO_DELETE_FUNCTION(TypeName(const TypeName&));            \
    TURBO_DELETE_FUNCTION(void operator=(const TypeName&))
#endif

// An older, deprecated, politically incorrect name for the above.
// NOTE: The usage of this macro was banned from our code base, but some
// third_party libraries are yet using it.
// TODO(tfarina): Figure out how to fix the usage of this macro in the
// third_party libraries and get rid of it.
#ifndef TURBO_DISALLOW_EVIL_CONSTRUCTORS
#define TURBO_DISALLOW_EVIL_CONSTRUCTORS(TypeName) TURBO_DISALLOW_COPY_AND_ASSIGN(TypeName)
#endif

// A macro to disallow all the implicit constructors, namely the
// default constructor, copy constructor and operator= functions.
//
// This should be used in the private: declarations for a class
// that wants to prevent anyone from instantiating it. This is
// especially useful for classes containing only static methods.
#ifndef TURBO_DISALLOW_IMPLICIT_CONSTRUCTORS
#define TURBO_DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName) \
    TURBO_DELETE_FUNCTION(TypeName());            \
    TURBO_DISALLOW_COPY_AND_ASSIGN(TypeName)
#endif

#ifndef TURBO_DEFAULT_MOVE_AND_ASSIGN
#define TURBO_DEFAULT_MOVE_AND_ASSIGN(TypeName) \
  TypeName(TypeName&&) = default;               \
  TypeName& operator=(TypeName&&) = default
#endif

namespace turbo {
    template<typename T>
    constexpr void ignore_result(const T &) {
    }
} // namespace turbo

#define TURBO_UNUSED(x) turbo::ignore_result((x))


// The following enum should be used only as a constructor argument to indicate
// that the variable has static storage class, and that the constructor should
// do nothing to its state.  It indicates to the reader that it is legal to
// declare a static instance of the class, provided the constructor is given
// the turbo::LINKER_INITIALIZED argument.  Normally, it is unsafe to declare a
// static variable that has a constructor or a destructor because invocation
// order is undefined.  However, IF the type can be initialized by filling with
// zeroes (which the loader does for static variables), AND the destructor also
// does nothing to the storage, AND there are no virtual methods, then a
// constructor declared as
//       explicit MyClass(turbo::LinkerInitialized x) {}
// and invoked as
//       static MyClass my_variable_name(turbo::LINKER_INITIALIZED);
namespace turbo {
    enum LinkerInitialized {
        LINKER_INITIALIZED
    };

// Use these to declare and define a static local variable (static T;) so that
// it is leaked so that its destructors are not called at exit. If you need
// thread-safe initialization, use lazy_instance.h instead.
#undef TURBO_CR_DEFINE_STATIC_LOCAL
#define TURBO_CR_DEFINE_STATIC_LOCAL(type, name, arguments) \
  static type& name = *new type arguments

}  // namespace turbo

// ptr:     the pointer to the member.
// type:    the type of the container struct this is embedded in.
// member:  the name of the member within the struct.
#ifndef kumo_container_of
# define kumo_container_of(ptr, type, member) ({                             \
            const decltype( ((type *)0)->member ) *__mptr = (ptr);  \
            (type *)( (char *)__mptr - offsetof(type,member) );})
#endif

// TURBO_DEFINE_SMALL_ARRAY(MyType, my_array, size, 64);
//   my_array is typed `MyType*' and as long as `size'. If `size' is not
//   greater than 64, the array is allocated on stack.
//
// NOTE: NEVER use TURBO_ARRAYSIZE(my_array) which is always 1.

#if defined(__cplusplus)
namespace turbo {
    namespace internal {
        template<typename T>
        struct ArrayDeleter {
            ArrayDeleter() : arr(0) {}

            ~ArrayDeleter() { delete[] arr; }

            T *arr;
        };
    }
}

// Many versions of clang does not support variable-length array with non-pod
// types, have to implement the macro differently.
#if !defined(__clang__)
# define TURBO_DEFINE_SMALL_ARRAY(Tp, name, size, maxsize)                    \
    Tp* name = 0;                                                       \
    const unsigned name##_size = (size);                                \
    const unsigned name##_stack_array_size = (name##_size <= (maxsize) ? name##_size : 0); \
    Tp name##_stack_array[name##_stack_array_size];                     \
    ::turbo::internal::ArrayDeleter<Tp> name##_array_deleter;            \
    if (name##_stack_array_size) {                                      \
        name = name##_stack_array;                                      \
    } else {                                                            \
        name = new (::std::nothrow) Tp[name##_size];                    \
        name##_array_deleter.arr = name;                                \
    }
#else
// This implementation works for GCC as well, however it needs extra 16 bytes
// for ArrayCtorDtor.
namespace turbo {
namespace internal {
template <typename T> struct ArrayCtorDtor {
    ArrayCtorDtor(void* arr, unsigned size) : _arr((T*)arr), _size(size) {
        for (unsigned i = 0; i < size; ++i) { new (_arr + i) T; }
    }
    ~ArrayCtorDtor() {
        for (unsigned i = 0; i < _size; ++i) { _arr[i].~T(); }
    }
private:
    T* _arr;
    unsigned _size;
};
}}
# define TURBO_DEFINE_SMALL_ARRAY(Tp, name, size, maxsize)                    \
    Tp* name = 0;                                                       \
    const unsigned name##_size = (size);                                \
    const unsigned name##_stack_array_size = (name##_size <= (maxsize) ? name##_size : 0); \
    char name##_stack_array[sizeof(Tp) * name##_stack_array_size];      \
    ::turbo::internal::ArrayDeleter<char> name##_array_deleter;          \
    if (name##_stack_array_size) {                                      \
        name = (Tp*)name##_stack_array;                                 \
    } else {                                                            \
        name = (Tp*)new (::std::nothrow) char[sizeof(Tp) * name##_size];\
        name##_array_deleter.arr = (char*)name;                         \
    }                                                                   \
    const ::turbo::internal::ArrayCtorDtor<Tp> name##_array_ctor_dtor(name, name##_size);
#endif // !defined(__clang__)
#endif // defined(__cplusplus)

// Put following code somewhere global to run it before main():
//
//   TURBO_GLOBAL_INIT()
//   {
//       ... your code ...
//   }
//
// Your can:
//   * Write any code and access global variables.
//   * Use ASSERT_*.
//   * Have multiple TURBO_GLOBAL_INIT() in one scope.
//
// Since the code run in global scope, quit with exit() or similar functions.

#if defined(__cplusplus)
# define TURBO_GLOBAL_INIT                                      \
namespace {  /*anonymous namespace */                           \
    struct TURBO_CONCAT(KumoGlobalInit, __LINE__) {            \
        TURBO_CONCAT(KumoGlobalInit, __LINE__)() { init(); }   \
        void init();                                            \
    } TURBO_CONCAT(kumo_global_init_dummy_, __LINE__);         \
}  /* anonymous namespace */                                    \
    void TURBO_CONCAT(KumoGlobalInit, __LINE__)::init
#else
# define TURBO_GLOBAL_INIT                      \
    static void __attribute__((constructor))    \
    TURBO_CONCAT(kumo_global_init_, __LINE__)

#endif  // __cplusplus

#ifndef TURBO_RESTRICT
#if defined(__clang__) || defined(__GNUC__)
#define TURBO_RESTRICT __restrict__
#elif defined(_MSC_VER)
#define TURBO_RESTRICT __restrict
#else
#define TURBO_RESTRICT
#endif
#endif

#ifndef TURBO_DISABLE_UBSAN
#if defined(__clang__)
#define TURBO_DISABLE_UBSAN(feature) __attribute__((no_sanitize(feature)))
#else
#define TURBO_DISABLE_UBSAN(feature)
#endif

#endif  // TURBO_DISABLE_UBSAN


// With TURBO_UNLIKELY, GCC and clang can be told that a certain branch is
// not likely to be taken (for instance, a KCHECK failure), and use that information in
// static analysis. Giving the compiler this information can affect the generated code
// layout in the absence of better information (i.e. -fprofile-arcs). [1] explains how
// this feature can be used to improve code generation. It was written as a positive
// comment to a negative article about the use of these annotations.
//
// TURBO_COMPILER_ASSUME allows the compiler to assume that a given expression is
// true, without evaluating it, and to optimise based on this assumption [2]. If this
// condition is violated at runtime, the behavior is undefined. This can be useful to
// generate both faster and smaller code in compute kernels.
//
// IMPORTANT: Different optimisers are likely to react differently to this annotation!
// It should be used with care when we can prove by some means that the assumption
// is (1) guaranteed to always hold and (2) is useful for optimization [3]. If the
// assumption is pessimistic, it might even block the compiler from decisions that
// could lead to better code [4]. If you have a good intuition for what the compiler
// can do with assumptions [5], you can use this macro to guide it and end up with
// results you would only get with more complex code transformations.
// `clang -S -emit-llvm` can be used to check how the generated code changes with
// your specific use of this macro.
//
// [1] https://lobste.rs/s/uwgtkt/don_t_use_likely_unlikely_attributes#c_xi3wmc
// [2] "Portable assumptions"
//     https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p1774r4.pdf
// [3] "Assertions Are Pessimistic, Assumptions Are Optimistic"
//     https://blog.regehr.org/archives/1096
// [4] https://discourse.llvm.org/t/llvm-assume-blocks-optimization/71609
// [5] J. Doerfert et al. 2019. "Performance Exploration Through Optimistic Static
//     Program Annotations". https://github.com/jdoerfert/PETOSPA/blob/master/ISC19.pdf
#ifdef TURBO_WARN_DOCUMENTATION
#define TURBO_ARG_UNUSED(x) x
#else
#define TURBO_ARG_UNUSED(x)
#endif

// ----------------------------------------------------------------------

// macros to disable padding
// these macros are portable across different compilers and platforms
//[https://github.com/google/flatbuffers/blob/master/include/flatbuffers/flatbuffers.h#L1355]
#if !defined(TURBO_MANUALLY_ALIGNED_STRUCT)
#if defined(_MSC_VER)
#define TURBO_MANUALLY_ALIGNED_STRUCT(alignment) \
  __pragma(pack(1));                       \
  struct __declspec(align(alignment))
#define STRUCT_END(name, size) \
  __pragma(pack());            \
  static_assert(sizeof(name) == size, "compiler breaks packing rules")
#elif defined(__GNUC__) || defined(__clang__)
#define TURBO_MANUALLY_ALIGNED_STRUCT(alignment) \
  _Pragma("pack(1)") struct __attribute__((aligned(alignment)))
#define STRUCT_END(name, size) \
  _Pragma("pack()") static_assert(sizeof(name) == size, "compiler breaks packing rules")
#else
#error Unknown compiler, please define structure alignment macros
#endif
#endif  // !defined(TURBO_MANUALLY_ALIGNED_STRUCT)