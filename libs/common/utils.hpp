// Copyright (c) 2018 Bronislaw (Bronek) Kozicki
//
// Distributed under the MIT License. See accompanying file LICENSE
// or copy at https://opensource.org/licenses/MIT

#pragma once

#include <type_traits>
#include <utility>

// Allow custom ASSERT macro
#ifndef ASSERT
# include <cassert>
# define ASSERT(...) assert((__VA_ARGS__))
#endif

namespace common {
    namespace impl {
        template <typename Type, bool DestructionPolicy> struct destroy_impl;

        template <typename Type> struct destroy_impl<Type, true> {
            static_assert(std::is_trivially_destructible_v<Type>);
            static void fn(Type* ) noexcept { } // no-op
        };

        template <typename Type> struct destroy_impl<Type, false> {
            static void fn(Type* ptr) noexcept { ptr->~Type(); }
        };
    }

    template <typename Type> struct destroy {
        using type = typename std::remove_cv<typename std::remove_reference<Type>::type>::type;
        using impl = impl::destroy_impl<type, std::is_trivially_destructible_v<type>>;
        static void fn(Type* ptr) noexcept { impl::fn(ptr); }
    };

    namespace impl {
        template <typename Type> struct emplace_impl {
            using type = typename std::remove_cv<typename std::remove_reference<Type>::type>::type;

            template<typename Sentinel, typename ... Args>
            static type* fn(Sentinel* dst, Args &&... a) noexcept {
                destroy<type>::fn(dst); // This must fail during compilation if dst is wrong type or const
                return new(dst) type{std::forward<Args>(a)...};
            }

            template <typename ... Args>
            static type* fn(void* dst, Args&& ... a) noexcept {
                return new (dst) type{std::forward<Args>(a)...};
            }
        };
    }

    template <typename Type, typename ... Args>
    static Type* emplace(Type* dst, Args&& ... a) noexcept {
        return impl::emplace_impl<Type>::fn(dst, std::forward<Args>(a)...);
    }

    template <typename Type, typename ... Args>
    static Type* emplace(void* dst, Args&& ... a) noexcept {
        return impl::emplace_impl<Type>::fn(dst, std::forward<Args>(a)...);
    }
} // namespace common
