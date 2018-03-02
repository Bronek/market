#pragma once

#include <utility>
#include <cstddef>
#include <stdexcept>

namespace market {
    // Also used for indexing, so lets give it appropriate underlying type
    enum class side : size_t { bid = 0, ask = 1 };

    template <typename LevelType> struct book;

    template <typename LevelType>
    struct book {
        // Actual level type, pulled from template parameters
        using level = LevelType;

        // This class is non-assignable
        book& operator=(const book& ) = delete;

        // Can be thrown by constructor, unless nothrow_empty_t or nothrow_t overload used
        struct bad_capacity : std::runtime_error {
            explicit bad_capacity(int i)
                : std::runtime_error("invalid market book capacity " + std::to_string(i))
            { }
        };

        // This structure can store at most 127 levels on each side, i.e. 254 in total
        // Books which require larger depth will have to to use a different data type
        using size_type = uint8_t;
        constexpr static size_type npos = 255;

        // Used for overloading of constructors
        using nothrow_t = std::nothrow_t;
        constexpr static nothrow_t nothrow{};

    private:
        // Functions defined below are documentation of a contract between this class and level type
        // Disallow exceptions, because this enables more aggressive optimisations.

        // Selected as appropriate by push_back()
        static void assign(level& dest, level&& src) noexcept { dest = std::move(src); }
        static void assign(level& dest, const level& src) noexcept { dest = src; }

        // Allow emplace-like assignment, if we are not concerned about destruction of old data
        template <typename ... Args>
        static void assign(level& dest, Args&& ... a) noexcept {
            static_assert(std::is_trivially_destructible_v<level>);
            new (&dest) level{std::forward<Args>(a)...};
        }

    protected:
        // Size of "levels" and "sides" arrays must NOT be smaller than "capacity * 2"
        level*          levels; // Array where levels are stored
        size_type*      sides; // 2D array of indices in levels, first by side
        size_type       level_i; // Total number of levels allocated from levels array
        size_type       side_i[2]; // Actual number of levels present on each side
        const size_type max_i; // Total capacity of "levels", i.e. capacity * 2

        // Safe to use "capacity" = 0, and just useful enough to report that the container is useless
        constexpr book(level* l, size_type* s, int d, const nothrow_t)
                : levels(l), sides(s), level_i(0), side_i{0, 0}, max_i((size_type)(d * 2))
                , capacity((d < 0 || d > 127) ? 0 : (size_type)d)
        { }

        // Safe to initialise "capacity" to 0, even though not very useful
        book(level* l, size_type* s, int d)
                : levels(l), sides(s), level_i(0), side_i{0, 0}, max_i((size_type)(d * 2)), capacity((size_type)d)
        {
            if (d < 0 || d > 127) {
                throw bad_capacity(d);
            }
        }

        // Class "data" does not have to be used, but it helps. Obviously it cannot
        // be used when capacity is determined in runtime (or is 0), in which case the
        // derived class has to take care of memory management for both tables.
        template <int Size>
        struct data {
            static_assert(Size > 0 && Size <= 127);
            constexpr static size_type capacity = (size_type)Size;
            LevelType levels[Size * 2];
            size_type sides[Size * 2];
        };

        template <int Size>
        explicit constexpr book(data<Size>& p)
                : levels(p.levels), sides(p.sides), level_i(0), side_i{0, 0}, max_i(p.capacity * (size_type)2)
                , capacity(p.capacity)
        { }

    public:
        const size_type capacity;

        template <side Side, typename ... Args>
        size_type push_back(Args&& ... a) {
            size_type result = npos;
            auto& i = side_i[(size_t)Side];
            if (i < capacity && level_i < max_i) {
                const auto l = level_i++; // Note: must post-increment level_i here
                assign(levels[l], std::forward<Args>(a)...);
                sides[(size_t)Side * capacity + i] = l;
                result = i++; // Note: must post-increment side_i[Side] here
            }
            return result;
        }

        template <side Side>
        size_type size() const {
            return side_i[(size_t)Side];
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
            const auto l = sides[(size_t)Side * capacity + i];
            return levels[l];
        }

        template <side Side>
        level& at(size_type i) {
            const auto l = sides[(size_t)Side * capacity + i];
            return levels[l];
        }
    };
}
