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
// AlignedMemory is a POD type that gives you a portable way to specify static
// or local stack data of a given alignment and size. For example, if you need
// static storage for a class, but you want manual control over when the object
// is constructed and destructed (you don't want static initialization and
// destruction), use AlignedMemory:
//
//   static AlignedMemory<sizeof(MyClass), ALIGNOF(MyClass)> my_class;
//
//   // ... at runtime:
//   new(my_class.void_data()) MyClass();
//
//   // ... use it:
//   MyClass* mc = my_class.data_as<MyClass>();
//
//   // ... later, to destruct my_class:
//   my_class.data_as<MyClass>()->MyClass::~MyClass();
//
// Alternatively, a runtime sized aligned allocation can be created:
//
//   float* my_array = static_cast<float*>(AlignedAlloc(size, alignment));
//
//   // ... later, to release the memory:
//   AlignedFree(my_array);
//
// Or using scoped_ptr:
//
//   scoped_ptr<float, AlignedFreeDeleter> my_array(
//       static_cast<float*>(AlignedAlloc(size, alignment)));

#pragma once

#include <turbo/base/macros.h>

#if defined(COMPILER_MSVC)
#include <malloc.h>
#else

#include <stdlib.h>

#endif

namespace turbo {

    // AlignedMemory is specialized for all supported alignments.
    // Make sure we get a compiler error if someone uses an unsupported alignment.
    template<size_t Size, size_t ByteAlignment>
    struct AlignedMemory {
    };

#define TURBO_DECL_ALIGNED_MEMORY(byte_alignment) \
    template <size_t Size> \
    class AlignedMemory<Size, byte_alignment> { \
     public: \
      ALIGNAS(byte_alignment) uint8_t data_[Size]; \
      void* void_data() { return static_cast<void*>(data_); } \
      const void* void_data() const { \
        return static_cast<const void*>(data_); \
      } \
      template<typename Type> \
      Type* data_as() { return static_cast<Type*>(void_data()); } \
      template<typename Type> \
      const Type* data_as() const { \
        return static_cast<const Type*>(void_data()); \
      } \
     private: \
      void* operator new(size_t); \
      void operator delete(void*); \
    }

    // Specialization for all alignments is required because MSVC (as of VS 2008)
    // does not understand ALIGNAS(ALIGNOF(Type)) or ALIGNAS(template_param).
    // Greater than 4096 alignment is not supported by some compilers, so 4096 is
    // the maximum specified here.
    TURBO_DECL_ALIGNED_MEMORY(1);

    TURBO_DECL_ALIGNED_MEMORY(2);

    TURBO_DECL_ALIGNED_MEMORY(4);

    TURBO_DECL_ALIGNED_MEMORY(8);

    TURBO_DECL_ALIGNED_MEMORY(16);

    TURBO_DECL_ALIGNED_MEMORY(32);

    TURBO_DECL_ALIGNED_MEMORY(64);

    TURBO_DECL_ALIGNED_MEMORY(128);

    TURBO_DECL_ALIGNED_MEMORY(256);

    TURBO_DECL_ALIGNED_MEMORY(512);

    TURBO_DECL_ALIGNED_MEMORY(1024);

    TURBO_DECL_ALIGNED_MEMORY(2048);

    TURBO_DECL_ALIGNED_MEMORY(4096);

#undef TURBO_DECL_ALIGNED_MEMORY

    TURBO_EXPORT void *AlignedAlloc(size_t size, size_t alignment);

    inline void AlignedFree(void *ptr) {
#if defined(COMPILER_MSVC)
        _aligned_free(ptr);
#else
        free(ptr);
#endif
    }

    // Deleter for use with scoped_ptr. E.g., use as
    //   scoped_ptr<Foo, turbo::AlignedFreeDeleter> foo;
    struct AlignedFreeDeleter {
        inline void operator()(void *ptr) const {
            AlignedFree(ptr);
        }
    };

}  // namespace turbo
