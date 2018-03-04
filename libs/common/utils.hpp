#pragma once

#include <type_traits>
#include <utility>

namespace common {
    template <typename Type, bool DestructionPolicy> struct destroy_impl;

    template <typename Type> struct destroy_impl<Type, true> {
        static_assert(std::is_trivially_destructible_v<Type>);
        static void fn(Type* ) noexcept { } // no-op
    };

    template <typename Type> struct destroy_impl<Type, false> {
        static void fn(Type* ptr) noexcept { ptr->~Type(); }
    };

    template <typename Type> struct destroy {
        using type = typename std::remove_cv<typename std::remove_reference<Type>::type>::type;
        using impl = destroy_impl<type, std::is_trivially_destructible_v<type>>;
        static void fn(Type* ptr) noexcept { impl::fn(ptr); }
    };

    template <typename Type> struct emplace {
        using type = typename std::remove_cv<typename std::remove_reference<Type>::type>::type;

        template <typename Sentinel, typename ... Args>
        static type& fn(Sentinel* dst, Args&& ... a) noexcept {
            using sent = typename std::remove_volatile<typename std::remove_reference<Sentinel>::type>::type;
            static_assert(not std::is_const_v<sent>, "Overwriting of const data");
            static_assert(std::is_same_v<sent, type>, "Unexpected pointer type, intentionally captured by overloading");
            destroy<type>::fn(dst);
            return *(new (dst) Type{std::forward<Args>(a)...});
        }

        template <typename ... Args>
        static type& fn(void* dst, Args&& ... a) noexcept {
            return *(new (dst) Type{std::forward<Args>(a)...});
        }
    };
} // namespace common
