// Copyright (c) 2018 Bronislaw (Bronek) Kozicki
//
// Distributed under the MIT License. See accompanying file LICENSE
// or copy at https://opensource.org/licenses/MIT

#pragma once

#include "common/utils.hpp"
#include "market.hpp"

#include <utility>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <algorithm>

namespace market {
    template <typename Level, typename Policy = Level>
    struct book {
        // Actual level type, pulled from template parameters
        using level = typename std::remove_cv<typename std::remove_reference<Level>::type>::type;

        // This class is non-assignable
        book& operator=(const book& ) = delete;

        // Can be thrown by constructor, unless nothrow_empty_t or nothrow_t overload used
        struct bad_capacity : std::runtime_error {
            explicit bad_capacity(int i)
                : std::runtime_error("invalid market book capacity " + std::to_string(i))
            { }
        };

        // This structure can store at most 127 levels on each side, i.e. 254 in total
        // Books which require larger depth will have to use a different data type
        using size_type = uint8_t;
        constexpr static size_type npos = 255;
        static_assert(npos == (size_type)(-1));
        static_assert((size_type)(npos + 1) == 0);

    private:
        // Functions sort() and binary_search() require "compare", which must be provided by the
        // Policy. The function must return true if level lh is closer to the top of the book than
        // level rh (on the given Side)
        template <side Side, typename Lh, typename Rh>
        static constexpr bool compare(Lh&& lh, Rh&& rh) noexcept {
            return Policy::template compare<Side>(std::forward<Lh>(lh), std::forward<Rh>(rh));
        }

        // Function binary_search() requires "make", which must be provided by the Policy. The
        // function must create a Level object (or suitable proxy) which will be used for comparison
        // when performing search
        template <typename ... Args>
        static constexpr auto make(Args&& ... a) noexcept {
            return Policy::make(std::forward<Args>(a)...);
        }

        template <side Side, typename Value>
        size_type upper_bound_(const size_type* begin,
                               const size_type* from,
                               const size_type* end,
                               const Value& val
        ) const {
            for (; from != end;) {
                const auto* mid = from + ((end - from) / 2);
                if (not book::compare<Side>(val, levels[*mid])) {
                    from = mid + 1;
                } else {
                    end = mid;
                }
            }
            const size_type ret = from - begin;
            if (ret == side_i[(size_t)Side]) {
                return npos;
            }
            return ret;
        }

        template <side Side, typename Value>
        size_type lower_bound_(const size_type* begin,
                               const size_type* from,
                               const size_type* end,
                               const Value& val
        ) const {
            for (; from != end;) {
                const auto* mid = from + ((end - from) / 2);
                if (book::compare<Side>(levels[*mid], val)) {
                    from = mid + 1;
                } else {
                    end = mid;
                }
            }
            const size_type ret = from - begin;
            if (ret == side_i[(size_t)Side]) {
                return npos;
            }
            return ret;
        }

    protected:
        // Size of "levels" "sides" and "freel" arrays must NOT be smaller than "capacity * 2"
        level*          levels; // Array where levels are stored
        size_type*      sides; // Array of indices in levels, first half bids and second asks
        size_type*      freel; // Free list, i.e. all unallocated indices in levels
        const size_type size_i; // Size of all above arrays, i.e. capacity * 2
        size_type       tail_i; // Index of last element in free list
        size_type       side_i[2]; // Actual number of levels present on each side

        // The memory for the three arrays "levels" "sides" and "freel" must be owned and maintained
        // by the derived class.

        // Safe to initialise "capacity" to 0, even though not very useful
        book(level* l, size_type* s, size_type* f, int d, size_type b, size_type a)
            : levels(l)
            , sides(s)
            , freel(f)
            , size_i((size_type)(d * 2))
            , tail_i(size_i - (size_type)1 - b - a) // Note: will be npos if no space left
            , side_i{b, a}
            , capacity((size_type)d) {
            if (d < 0 || d > 127) {
                throw bad_capacity(d);
            }
        }

        // Used for overloading of constructors
        using nothrow_t = std::nothrow_t;
        constexpr static nothrow_t nothrow{};

        // Safe to use "capacity" = 0, and just useful enough to report that the container is useless
        constexpr book(level* l, size_type* s, size_type* f, int d, size_type b, size_type a, const nothrow_t)
            : levels(l)
            , sides(s)
            , freel(f)
            , size_i((size_type)(d * 2))
            , tail_i(size_i - (size_type)1 - b - a)
            , side_i{b, a}
            , capacity((d < 0 || d > 127) ? 0 : (size_type)d)
        { }

        // Can be used to construct immutable books (also 0 capacity)
        constexpr book(const level* l, const size_type* s, int d, size_type b, size_type a, const nothrow_t)
                : levels(const_cast<level*>(l))
                , sides(const_cast<size_type*>(s))
                , freel(nullptr) // see tail_i(npos) below
                , size_i((size_type)(d * 2))
                , tail_i(npos)
                , side_i{b, a}
                , capacity((d < 0 || d > 127) ? 0 : (size_type)d)
        { }

        // Class "data" does not have to be used, but it helps. Obviously it cannot
        // be used when capacity is determined in runtime (or is 0), in which case the
        // derived class has to take care of memory management for both tables.
        template <int Size>
        struct data {
            static_assert(Size > 0 && Size <= 127);
            constexpr static size_type capacity = (size_type)Size;
            level levels[Size * 2] = {};
            size_type sides[Size * 2] = {};
            size_type freel[Size * 2] = {};
        };

        template <int Size>
        constexpr explicit book(data<Size>& p, size_type b, size_type a)
            : levels(p.levels)
            , sides(p.sides)
            , freel(p.freel)
            , size_i(p.capacity * (size_type)2)
            , tail_i(size_i - (size_type)1 - b - a)
            , side_i{b, a}
            , capacity(p.capacity)
        { }

        template <int Size>
        constexpr explicit book(const data<Size>& p, size_type b, size_type a)
                : levels(p.levels)
                , sides(p.sides)
                , freel(p.freel)
                , size_i(p.capacity * (size_type)2)
                , tail_i(size_i - (size_type)1 - b - a)
                , side_i{b, a}
                , capacity(p.capacity)
        { }

        // If freel is not populated to match tail_i, the derived class must call either of the
        // initialisation functions reset() or accept() to populate it.
        void reset() {
            ASSERT(freel != nullptr);
            size_type i = 0;
            for (; i < size_i; ++i) {
                freel[i] = i;
            }
            tail_i = size_i - 1;
            side_i[0] = side_i[1] = 0;
        };

        void accept() {
            ASSERT(freel != nullptr);
            for (size_type i = 0; i < size_i; ++i) {
                freel[i] = i;
            }
            // Mark elements in the free list, which are not actually free, with npos
            for (size_type i = 0; i < side_i[0] || i < side_i[1]; ++i) {
                if (i < side_i[0]) {
                    freel[sides[i]] = npos;
                }
                if (i < side_i[1]) {
                    freel[sides[capacity + i]] = npos;
                }
            }
            // Note: tail_i will underflow to npos if no space left, by design
            tail_i = size_i - (size_type)1 - side_i[0] - side_i[1];
            // Shift freel elements marked as taken (i.e. with npos value) to the end of freel
            // Note: npos will not be preserved, it has no special meaning outside of accept
            auto* const begin = &freel[0];
            std::remove_if(begin, begin + size_i, [](size_type n){ return n == npos; } );
        };

    public:
        const size_type capacity;

        template <side Side, typename Type>
        size_type push_back(Type&& a) {
            ASSERT(freel != nullptr);
            ASSERT(side_i[0] + side_i[1] + (size_type)(tail_i + 1) == size_i);
            size_type result = npos;
            auto& i = side_i[(size_t)Side];
            if (i < capacity && tail_i != npos) {
                // Note: must post-decrement tail_i here. Will change to npos if it was 0
                const auto l = freel[tail_i--];
                levels[l] = std::forward<Type>(a);
                sides[(size_t)Side * capacity + i] = l;
                result = i++; // Note: must post-increment side_i[Side] here
            }
            return result;
        }

        template <side Side, typename ... Args>
        size_type emplace_back(Args&& ... a) {
            ASSERT(freel != nullptr);
            ASSERT(side_i[0] + side_i[1] + (size_type)(tail_i + 1) == size_i);
            size_type result = npos;
            auto& i = side_i[(size_t)Side];
            if (i < capacity && tail_i != npos) {
                // Note: must post-decrement tail_i here. Will change to npos if it was 0
                const auto l = freel[tail_i--];
                common::emplace(&levels[l], std::forward<Args>(a) ...);
                sides[(size_t)Side * capacity + i] = l;
                result = i++; // Note: must post-increment side_i[Side] here
            }
            return result;
        }

        template <side Side>
        void remove(size_type i) {
            ASSERT(freel != nullptr);
            ASSERT(i < side_i[(size_t)Side]);
            ASSERT(side_i[0] + side_i[1] + (size_type)(tail_i + 1) == size_i);
            const auto l = sides[(size_t)Side * capacity + i];
            freel[++tail_i] = l; // Note: must pre-increment tail_l here
            const auto size = (side_i[(size_t)Side])--; // Note: must post-decrement side[Side]
            for (size_type j = i; j < size;) {
                auto& n = sides[(size_t)Side * capacity + j];
                n = sides[(size_t)Side * capacity + ++j]; // Note: must pre-increment j here
            }
        }

        template <side Side>
        size_type size() const {
            return side_i[(size_t)Side];
        }

        template <side Side>
        void sort() {
            auto* const begin = &sides[(size_t)Side * capacity];
            std::sort(begin, begin + side_i[(size_t)Side], [this](size_type lh, size_type rh){
                return book::compare<Side>(levels[lh], levels[rh]);
            });
        }

        template <side Side>
        bool empty() const {
            return side_i[(size_t)Side] == 0;
        }

        template <side Side>
        bool full() const {
            return side_i[(size_t)Side] == capacity;
        }

        template <side Side>
        const level& at(size_type i) const {
            ASSERT(i < side_i[(size_t)Side]);
            const auto l = sides[(size_t)Side * capacity + i];
            return levels[l];
        }

        template <side Side>
        level& at(size_type i) {
            ASSERT(i < side_i[(size_t)Side]);
            const auto l = sides[(size_t)Side * capacity + i];
            return levels[l];
        }

        template <side Side, typename ... Args>
        size_type binary_search(Args &&... a) const {
            const auto& val = book::make(std::forward<Args>(a)...);
            const auto* begin = &sides[(size_t)Side * capacity];
            const auto size = side_i[(size_t)Side];
            const auto* end = begin + size;
            // Cannot use std::binary_search here, because that wouldn't have returned an index
            // Note, this implementation is searching in a half-closed ranges. It will never refer
            // to "end" element, hence "end = mid" after we have checked the middle element.
            for (const auto* from = begin; from != end;) {
                const auto* mid = from + ((end - from) / 2);
                const bool tmp = book::compare<Side>(val, levels[*mid]);
                if (not tmp && not book::compare<Side>(levels[*mid], val)) {
                    return size_type(mid - begin); // Found it!
                } else if (tmp) {
                    end = mid;
                } else {
                    from = mid + 1;
                }
            }
            return npos;
        }

        template <side Side, typename ... Args>
        size_type lower_bound(Args&& ... a) const {
            const auto& val = book::make(std::forward<Args>(a)...);
            const auto* begin = &sides[(size_t)Side * capacity];
            const auto* end = begin + side_i[(size_t)Side];
            return lower_bound_<Side>(begin, begin, end, val);
        }

        template <side Side, typename ... Args>
        size_type upper_bound(Args&& ... a) const {
            const auto& val = book::make(std::forward<Args>(a)...);
            const auto* begin = &sides[(size_t)Side * capacity];
            const auto* end = begin + side_i[(size_t)Side];
            return upper_bound_<Side>(begin, begin, end, val);
        }

        template <side Side, typename ... Args>
        std::pair<size_type, size_type> equal_range(Args&& ... a) const {
            const auto& val = book::make(std::forward<Args>(a)...);
            const auto* begin = &sides[(size_t)Side * capacity];
            const auto* end = begin + side_i[(size_t)Side];
            const auto* from = begin;
            for (; from != end;) {
                const auto* mid = from + ((end - from) / 2);
                if (book::compare<Side>(levels[*mid], val)) {
                    from = mid + 1;
                } else if (book::compare<Side>(val, levels[*mid])) {
                    end = mid;
                } else {
                    return std::make_pair(
                            lower_bound_<Side>(begin, from, mid, val),
                            upper_bound_<Side>(begin, mid + 1, end, val));
                }
            }
            size_type r = size_type(from - begin);
            if (r == side_i[(size_t)Side]) {
                return std::make_pair(npos, npos);
            }
            return std::make_pair(r, r);
        }
    };
} // namespace market
