// Copyright (c) 2018 Bronislaw (Bronek) Kozicki
//
// Distributed under the MIT License. See accompanying file LICENSE
// or copy at https://opensource.org/licenses/MIT

#pragma once

#include "market.hpp"
#include <common/utils.hpp>

#include <utility>
#include <cstddef>
#include <stdexcept>
#include <algorithm>

namespace market {
    template <typename Level>
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
        // We require the Level to provide function "better", which must return true if level lh
        // is closer to the top of the book than level rh (on the given Side)
        template <side Side>
        static constexpr bool compare(const level& lh, const level& rh) noexcept {
            return lh.template better<Side>(rh);
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
        // by the derived class. In the simplest variant, the derived class may delegate this task
        // to "template <int> struct data" defined below, but it may also use something different
        // like free store, own arrays, static data, data on stack, shared memory etc.
        // Actual data elements (i.e. levels) are stored in levels array, but they are usually
        // not referred to directly. Instead, the user will typically use an index into either half
        // of sides array (first half for bids, second for asks), and dereference of that index
        // will point to an index in levels. This level of indirection access schema makes it
        // possible to:
        // 1) quickly sort all levels, using sorting of indices stored in sides array This means
        //    that actual levels are not moved in memory during sorting
        // 2) quickly remove a level (or all levels on a side) by moving its index into levels array
        //    from sides array to free list (i.e. freel array). Again, no levels are moved in memory
        // 3) quickly append a level - by moving its index into levels from free list to sides
        //    array. Again no levels are moved in memory. If sorting is necessary after adding an
        //    element, it will be very efficient thanks to 1) above
        // 4) because levels do not move, they can be immutable (use emplace_back() function)
        // 5) because levels do not move, it is perfectly valid for the user to take the address of
        //    an element and refer to it later, even if other elements were removed/added in between
        //    or if the book was initially unsorted and has been sorted later. Neither of those
        //    operations will invalidate a pointer to a level stored in the book, as long as the
        //    level itself has not been removed. Note, that removing a level will decrement
        //    indices of levels following it, but will not change their location in memory
        // 6) because we operate at all times on 8bit small indices rather than pointers, most of
        //    the data accessed during operations is kept in a small region of memory, allowing for
        //    high cache hit ratio and good cache locality (hence low latency)
        // 7) finally, working on indices (rather than e.g. pointers) allows for the same data
        //    structure to be shared across process boundary as-is (no serialization necessary)
        //
        // Here is example memory layout of capacity 3 book, with 2 levels set on either side:
        // levels:
        //   {{data slot 0},{data slot 1},{data slot 2},{unused 3},{data slot 4},{unused 5}}
        // sides: (first 3 elements are bid indices, second 3 elements are ask indices)
        //   {0, 1, unused, 2, 4, unused} , side_i[] = {2, 2}
        // freel:
        //   {3, 5, unused, unused, unused, unused} , tail_i = 1
        // Sort will only change the order of elements in sides, for example:
        //   sides: {1, 0, unused, 2, 4, unused} , side_i[] = {2, 2}
        // Removing an element will move an index from sides to freel, for example:
        //   sides: {0, unused, unused, 2, 4, unused} , side_i[] = {1, 2}
        //   freel: {3, 5, 1, unused, unused, unused} , tail_i = 2
        // Adding an element will move index back again (in this example to "asks"):
        //   sides: {0, unused, unused, 2, 4, 1} , side_i[] = {1, 3}
        //   freel: {3, 5, unused, unused, unused, unused} , tail_i = 1
        // Adding one more element:
        //   sides: {0, 5, unused, 2, 4, 1} , side_i[] = {2, 3}
        //   freel: {3, unused, unused, unused, unused, unused} , tail_i = 0
        // Adding last element (attempts to add more elements will return npos i.e. "no space"):
        //   sides: {0, 5, 3, 2, 4, 1} , side_i[] = {3, 3}
        //   freel: {unused, unused, unused, unused, unused, unused} , tail_i = npos
        // After another sort (for some values in slot data, which we cannot see here):
        //   sides: {5, 0, 3, 1, 2, 4}
        // At no point did the data in levels array change or move. However, of course stale data
        // have been replaced (e.g. slot 1 in the example above), when elements were added.
        //
        // It is possible to remove the free list, and use (currently not used) slots in sides array
        // beyond sides_i[] instead. This would, however, come at the cost of complex conditional
        // instructions, i.e. multiple branches in the hot path of the execution. Such a solution
        // is likely to be less optimal than accessing one (typically) or at most four more cache
        // lines required to keep free list in memory.

        // Safe to initialise "capacity" to 0, even though not very useful
        book(level* l, size_type* s, size_type* f, int d, size_type b = 0, size_type a = 0)
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
        constexpr book(level* l, size_type* s, size_type* f, int d, const nothrow_t, size_type b = 0, size_type a = 0)
            : levels(l)
            , sides(s)
            , freel(f)
            , size_i((size_type)(d * 2))
            , tail_i(size_i - (size_type)1 - b - a)
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
        constexpr explicit book(data<Size>& p, size_type b = 0, size_type a = 0)
            : levels(p.levels)
            , sides(p.sides)
            , freel(p.freel)
            , size_i(p.capacity * (size_type)2)
            , tail_i(size_i - (size_type)1 - b - a)
            , side_i{b, a}
            , capacity(p.capacity)
        { }

        template <int Size>
        constexpr explicit book(const data<Size>& p, size_type b = 0, size_type a = 0)
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
            size_type i = 0;
            for (; i < size_i; ++i) {
                freel[i] = i;
            }
            tail_i = --i;
            side_i[0] = side_i[1] = 0;
            ASSERT(side_i[0] + side_i[1] + (size_type)(tail_i + 1) == size_i);
        };

        void accept() {
            size_type i = 0;
            for (; i < size_i; ++i) {
                freel[i] = i;
            }
            for (i = 0; i < side_i[0] || i < side_i[1]; ++i) {
                if (i < side_i[0]) {
                    freel[sides[i]] = npos;
                }
                if (i < side_i[1]) {
                    freel[sides[capacity + i]] = npos;
                }
            }
            // Shift freel elements marked as taken (i.e. with npos value) to the end of freel
            std::remove_if( &freel[0], &freel[0] + size_i, [](size_type n){ return n == npos; } );
            ASSERT(side_i[0] + side_i[1] + (size_type)(tail_i + 1) == size_i);
        };

    public:
        const size_type capacity;

        template <side Side, typename Type>
        size_type push_back(Type&& a) {
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
            std::sort(begin, begin + side_i[(size_t)Side], [this](size_t lh, size_t rh){
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
    };
} // namespace market
