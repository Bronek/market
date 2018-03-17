// Copyright (c) 2018 Bronislaw (Bronek) Kozicki
//
// Distributed under the MIT License. See accompanying file LICENSE
// or copy at https://opensource.org/licenses/MIT

#include <market/book.hpp>

#include <catch2/catch.hpp>

#include <set>
#include <vector>

namespace {
    struct Level {
        int ticks; int size;

        template <market::side Side>
        constexpr static bool compare(const Level& lh, const Level& rh) noexcept;

        template <market::side Side>
        constexpr static bool compare(int, const Level& rh) noexcept;

        template <market::side Side>
        constexpr static bool compare(const Level& lh, int) noexcept;

        constexpr static int make(int i) {
            return i;
        }
    };

    template <>
    constexpr bool Level::compare<market::side::bid>(const Level& lh, const Level& rh) noexcept {
        return lh.ticks > rh.ticks;
    }

    template <>
    constexpr bool Level::compare<market::side::bid>(int lh, const Level& rh) noexcept {
        return lh > rh.ticks;
    }

    template <>
    constexpr bool Level::compare<market::side::bid>(const Level& lh, int rh) noexcept {
        return lh.ticks > rh;
    }

    template <>
    constexpr bool Level::compare<market::side::ask>(const Level& lh, const Level& rh) noexcept {
        return lh.ticks < rh.ticks;
    }

    template <>
    constexpr bool Level::compare<market::side::ask>(int lh, const Level& rh) noexcept {
        return lh < rh.ticks;
    }

    template <>
    constexpr bool Level::compare<market::side::ask>(const Level& lh, int rh) noexcept {
        return lh.ticks < rh;
    }

    bool operator==(const Level& lh, const Level& rh) {
        return lh.ticks == rh.ticks && lh.size == rh.size;
    }

    std::ostream& operator<<(std::ostream& lh, const Level& rh) {
        return (lh << '{' << rh.ticks << ',' << rh.size << '}');
    }

    struct SmallBook : market::book<Level> {
        SmallBook() : book<Level>(data, 0, 0) {
            reset();
        }

        book::data<3> data;
    };
}

TEST_CASE("SmallBook_basics", "[book][data][capacity][size][empty][full][at][push_back][emplace_back][sort][remove]") {
    using namespace market;
    SmallBook book;
    const int back = book.capacity * 2 - 1;

    SECTION("elements for each of data.levels, data.sides and data.freel equal to twice capacity") {
        REQUIRE(book.capacity == 3);
        REQUIRE(back == 5);
        REQUIRE(book.data.capacity == book.capacity);
        REQUIRE(sizeof(book.data.levels) / sizeof(book.data.levels[0]) == back + 1);
        REQUIRE(sizeof(book.data.sides) / sizeof(book.data.sides[0]) == back + 1);
        REQUIRE(sizeof(book.data.freel) / sizeof(book.data.freel[0]) == back + 1);
    }

    SECTION("book initialised empty") {
        CHECK(book.size<side::bid>() == 0);
        CHECK(book.empty<side::bid>());
        CHECK(not book.full<side::bid>());
        CHECK(book.size<side::ask>() == 0);
        CHECK(book.empty<side::ask>());
        CHECK(not book.full<side::ask>());

        CHECK(book.find<side::bid>(130130) == SmallBook::npos);
        CHECK(book.find<side::ask>(130130) == SmallBook::npos);
    }

    SECTION("push_back() similar elements on each side to fill the capacity") {
        REQUIRE(SmallBook::npos > 3);
        CHECK(book.push_back<side::bid>(Level{130130, 100}) == 0);
        CHECK(book.size<side::bid>() == 1);
        CHECK(not book.empty<side::bid>());
        CHECK(not book.full<side::bid>());
        CHECK(book.push_back<side::bid>(Level{130130, 100}) == 1);
        CHECK(book.size<side::bid>() == 2);
        CHECK(not book.full<side::bid>());
        CHECK(book.push_back<side::bid>(Level{130130, 100}) == 2);
        CHECK(book.size<side::bid>() == 3);
        CHECK(book.full<side::bid>());
        CHECK(book.push_back<side::bid>(Level{130133, 400}) == SmallBook::npos);
        CHECK(book.size<side::bid>() == 3);
        CHECK(book.full<side::bid>());
        CHECK(book.find<side::bid>(130130) != SmallBook::npos);
        CHECK(book.find<side::bid>(130133) == SmallBook::npos);

        CHECK(book.push_back<side::ask>(Level{130130, 100}) == 0);
        CHECK(book.size<side::ask>() == 1);
        CHECK(not book.empty<side::ask>());
        CHECK(not book.full<side::ask>());
        CHECK(book.push_back<side::ask>(Level{130130, 200}) == 1);
        CHECK(book.size<side::ask>() == 2);
        CHECK(not book.full<side::ask>());
        CHECK(book.push_back<side::ask>(Level{130130, 300}) == 2);
        CHECK(book.size<side::ask>() == 3);
        CHECK(book.full<side::ask>());
        CHECK(book.push_back<side::ask>(Level{130133, 800}) == SmallBook::npos);
        CHECK(book.size<side::ask>() == 3);
        CHECK(book.full<side::ask>());
        CHECK(book.find<side::ask>(130130) != SmallBook::npos);
        CHECK(book.find<side::ask>(130133) == SmallBook::npos);

        SECTION("each element stored faithfully inside data.levels array") {
            std::vector<const Level *> levels;
            CHECK(book.at<side::bid>(0) == Level{130130, 100});
            levels.emplace_back(&book.at<side::bid>(0));
            CHECK(book.at<side::bid>(1) == Level{130130, 100});
            levels.emplace_back(&book.at<side::bid>(1));
            CHECK(book.at<side::bid>(2) == Level{130130, 100});
            levels.emplace_back(&book.at<side::bid>(2));
            CHECK(book.at<side::ask>(0) == Level{130130, 100});
            levels.emplace_back(&book.at<side::ask>(0));
            CHECK(book.at<side::ask>(1) == Level{130130, 200});
            levels.emplace_back(&book.at<side::ask>(1));
            CHECK(book.at<side::ask>(2) == Level{130130, 300});
            levels.emplace_back(&book.at<side::ask>(2));
            REQUIRE(levels.size() == 6);
            for (const auto *l : levels) {
                CHECK(l >= &book.data.levels[0]);
                CHECK(l <= &book.data.levels[back]);
            }

            SECTION("each element in its own unique memory location") {
                std::set<const Level *> copy {levels.begin(), levels.end()};
                REQUIRE(copy.size() == 6);
            }
        }
    }

    SECTION("emplace_back() different elements on each side to fill the capacity") {
        REQUIRE(SmallBook::npos > 3);
        CHECK(book.emplace_back<side::bid>(130130, 100) == 0);
        CHECK(book.size<side::bid>() == 1);
        CHECK(not book.empty<side::bid>());
        CHECK(not book.full<side::bid>());
        CHECK(book.emplace_back<side::bid>(130131, 200) == 1);
        CHECK(book.size<side::bid>() == 2);
        CHECK(not book.full<side::bid>());
        CHECK(book.emplace_back<side::bid>(130132, 300) == 2);
        CHECK(book.size<side::bid>() == 3);
        CHECK(book.full<side::bid>());
        CHECK(book.emplace_back<side::bid>(130133, 400) == SmallBook::npos);
        CHECK(book.size<side::bid>() == 3);
        CHECK(book.full<side::bid>());

        CHECK(book.emplace_back<side::ask>(130130, 500) == 0);
        CHECK(book.size<side::ask>() == 1);
        CHECK(not book.empty<side::ask>());
        CHECK(not book.full<side::ask>());

        CHECK(book.emplace_back<side::ask>(130131, 600) == 1);
        CHECK(book.size<side::ask>() == 2);
        CHECK(not book.full<side::ask>());

        CHECK(book.emplace_back<side::ask>(130132, 700) == 2);
        CHECK(book.size<side::ask>() == 3);
        CHECK(book.full<side::ask>());
        CHECK(book.find<side::ask>(130130) == 0);
        CHECK(book.find<side::ask>(130131) == 1);
        CHECK(book.find<side::ask>(130132) == 2);
        CHECK(book.find<side::ask>(130133) == SmallBook::npos);

        CHECK(book.emplace_back<side::ask>(130133, 800) == SmallBook::npos);
        CHECK(book.size<side::ask>() == 3);
        CHECK(book.full<side::ask>());

        SECTION("each element stored faithfully inside data.levels array") {
            // The important feature of market::book is that none of the operations (tested below)
            // moves elements in memory. Hence, capture their addresses before we test these.
            std::vector<const Level*> levels;

            CHECK(book.at<side::bid>(0) == Level{130130, 100});
            levels.emplace_back(&book.at<side::bid>(0));
            CHECK(book.at<side::bid>(1) == Level{130131, 200});
            levels.emplace_back(&book.at<side::bid>(1));
            CHECK(book.at<side::bid>(2) == Level{130132, 300});
            levels.emplace_back(&book.at<side::bid>(2));
            CHECK(book.at<side::ask>(0) == Level{130130, 500});
            levels.emplace_back(&book.at<side::ask>(0));
            CHECK(book.at<side::ask>(1) == Level{130131, 600});
            levels.emplace_back(&book.at<side::ask>(1));
            CHECK(book.at<side::ask>(2) == Level{130132, 700});
            levels.emplace_back(&book.at<side::ask>(2));
            REQUIRE(levels.size() == 6);
            for (const auto* l : levels) {
                CHECK(l >= &book.data.levels[0]);
                CHECK(l <= &book.data.levels[back]);
            }

            SECTION("each element in its own unique memory location") {
                std::set<const Level *> copy {levels.begin(), levels.end()};
                REQUIRE(copy.size() == 6);
            }

            SECTION("sort() does not move elements, only their indices") {
                // Sorting of elements only changes their indices, but data does not move in memory.
                std::vector<Level> copy;
                for (const auto* l : levels) {
                    copy.emplace_back(*l);
                }
                REQUIRE(copy.size() == 6);
                book.sort<side::bid>();
                book.sort<side::ask>();
                for (size_t i = 0; i < 6; ++i) {
                    const auto tmp1 = levels[i];
                    const auto& tmp2 = copy[i];
                    CHECK(*tmp1 == tmp2);
                }

                // Index of best bid moved to front, but actual data in original memory location
                CHECK(book.at<side::bid>(0) == Level{130132, 300});
                CHECK(levels[2] == &book.at<side::bid>(0));

                CHECK(book.at<side::bid>(1) == Level{130131, 200});
                CHECK(levels[1] == &book.at<side::bid>(1));

                CHECK(book.at<side::bid>(2) == Level{130130, 100});
                CHECK(levels[0] == &book.at<side::bid>(2));

                CHECK(book.at<side::ask>(0) == Level{130130, 500});
                CHECK(levels[3] == &book.at<side::ask>(0));

                CHECK(book.at<side::ask>(1) == Level{130131, 600});
                CHECK(levels[4] == &book.at<side::ask>(1));

                CHECK(book.at<side::ask>(2) == Level{130132, 700});
                CHECK(levels[5] == &book.at<side::ask>(2));

                CHECK(book.find<side::bid>(130132) == 0);
                CHECK(book.find<side::bid>(130131) == 1);
                CHECK(book.find<side::bid>(130130) == 2);
                CHECK(book.find<side::bid>(130133) == SmallBook::npos);
            }

            SECTION("const overload of at() returns the same data as non-const") {
                const auto& cref = book;

                CHECK(cref.at<side::bid>(0) == Level{130130, 100});
                CHECK(levels[0] == &cref.at<side::bid>(0));

                CHECK(cref.at<side::bid>(1) == Level{130131, 200});
                CHECK(levels[1] == &cref.at<side::bid>(1));

                CHECK(cref.at<side::bid>(2) == Level{130132, 300});
                CHECK(levels[2] == &cref.at<side::bid>(2));

                CHECK(cref.at<side::ask>(0) == Level{130130, 500});
                CHECK(levels[3] == &cref.at<side::ask>(0));

                CHECK(cref.at<side::ask>(1) == Level{130131, 600});
                CHECK(levels[4] == &cref.at<side::ask>(1));

                CHECK(cref.at<side::ask>(2) == Level{130132, 700});
                CHECK(levels[5] == &cref.at<side::ask>(2));
            }

            SECTION("remove() does not move elements, only their indices") {
                // Removal of an element only changes the indices of the following elements,
                // but they do not move in memory.

                // Remove top element on bid side
                book.remove<side::bid>(0);
                CHECK(book.size<side::bid>() == 2);
                CHECK(not book.empty<side::bid>());
                CHECK(not book.full<side::bid>());

                CHECK(book.at<side::bid>(0) == Level{130131, 200});
                CHECK(levels[1] == &book.at<side::bid>(0));

                CHECK(book.at<side::bid>(1) == Level{130132, 300});
                CHECK(levels[2] == &book.at<side::bid>(1));

                // Other side is unaffected
                CHECK(book.size<side::ask>() == 3);
                CHECK(levels[3] == &book.at<side::ask>(0));
                CHECK(levels[4] == &book.at<side::ask>(1));
                CHECK(levels[5] == &book.at<side::ask>(2));

                // Remove mid element on ask side
                book.remove<side::ask>(1);
                CHECK(book.size<side::ask>() == 2);
                CHECK(not book.empty<side::ask>());
                CHECK(not book.full<side::ask>());

                CHECK(book.at<side::ask>(0) == Level{130130, 500});
                CHECK(levels[3] == &book.at<side::ask>(0));

                CHECK(book.at<side::ask>(1) == Level{130132, 700});
                CHECK(levels[5] == &book.at<side::ask>(1));
                CHECK(book.find<side::ask>(130131) == SmallBook::npos);

                // Other side is unaffected
                CHECK(levels[1] == &book.at<side::bid>(0));
                CHECK(levels[2] == &book.at<side::bid>(1));
            }

            SECTION("remove() all elements from top (front) works") {
                book.remove<side::bid>(0);
                book.remove<side::bid>(0);
                CHECK(book.size<side::bid>() == 1);
                CHECK(not book.empty<side::bid>());
                CHECK(not book.full<side::bid>());

                CHECK(book.at<side::bid>(0) == Level{130132, 300});
                CHECK(levels[2] == &book.at<side::bid>(0));

                // Remove last element on bid side
                book.remove<side::bid>(0);
                CHECK(book.size<side::bid>() == 0);
                CHECK(book.empty<side::bid>());

                // Other side is unaffected
                CHECK(book.size<side::ask>() == 3);
                CHECK(levels[3] == &book.at<side::ask>(0));
                CHECK(levels[4] == &book.at<side::ask>(1));
                CHECK(levels[5] == &book.at<side::ask>(2));
            }

            SECTION("remove() all elements from bottom (back) works") {
                book.remove<side::ask>(2);
                book.remove<side::ask>(1);
                CHECK(book.size<side::ask>() == 1);
                CHECK(not book.empty<side::ask>());
                CHECK(not book.full<side::ask>());

                CHECK(book.at<side::ask>(0) == Level{130130, 500});
                CHECK(levels[3] == &book.at<side::ask>(0));

                // Remove last element on ask side
                book.remove<side::ask>(0);
                CHECK(book.size<side::ask>() == 0);
                CHECK(book.empty<side::ask>());

                // Other side is unaffected
                CHECK(book.size<side::bid>() == 3);
                CHECK(levels[0] == &book.at<side::bid>(0));
                CHECK(levels[1] == &book.at<side::bid>(1));
                CHECK(levels[2] == &book.at<side::bid>(2));
            }

            SECTION("remove() actually frees space, which can be reused") {
                // Remove some elements first
                book.remove<side::bid>(0);
                book.remove<side::bid>(0);
                CHECK(book.size<side::bid>() == 1);
                book.remove<side::ask>(2);
                book.remove<side::ask>(0);
                CHECK(book.size<side::ask>() == 1);

                // Check remaining elements are unaffected
                CHECK(book.at<side::bid>(0) == Level{130132, 300});
                CHECK(levels[2] == &book.at<side::bid>(0));
                CHECK(book.at<side::ask>(0) == Level{130131, 600});
                CHECK(levels[4] == &book.at<side::ask>(0));

                // emplace_back() more elements until full
                CHECK(book.emplace_back<side::bid>(130140, 200) == 1);
                CHECK(book.size<side::bid>() == 2);
                CHECK(book.emplace_back<side::bid>(130141, 300) == 2);
                CHECK(book.size<side::bid>() == 3);
                CHECK(book.emplace_back<side::bid>(130142, 400) == SmallBook::npos);

                CHECK(book.emplace_back<side::ask>(130120, 600) == 1);
                CHECK(book.size<side::ask>() == 2);
                CHECK(book.emplace_back<side::ask>(130129, 700) == 2);
                CHECK(book.size<side::ask>() == 3);
                CHECK(book.emplace_back<side::ask>(130128, 800) == SmallBook::npos);

                // Check elements at the top unaffected
                CHECK(book.at<side::bid>(0) == Level{130132, 300});
                CHECK(levels[2] == &book.at<side::bid>(0));
                CHECK(book.at<side::ask>(0) == Level{130131, 600});
                CHECK(levels[4] == &book.at<side::ask>(0));
            }
        }
    }
}

namespace {
    struct AnySizeBook : market::book<Level> {
        // Normally these two will not be exposed
        using book::nothrow_t;
        using book::nothrow;

        // Maximum size allowed, required here because we are testing corner cases
        Level levels[254] = {};
        uint8_t sides[254] = {};
        uint8_t freel[254] = {};

        template <typename ... Args>
        AnySizeBook(int i, Args&& ... t)
                : book(levels, sides, freel, i, 0, 0, std::forward<Args>(t)...) {
            this->accept();
        }
    };
}

TEST_CASE("AnySizeBook_construction", "[book][capacity][construction][bad_capacity][nothrow]") {
    using namespace market;
    constexpr auto npos = AnySizeBook::npos;
    constexpr auto nothrow = AnySizeBook::nothrow;

    std::unique_ptr<AnySizeBook> ptr = nullptr;
    SECTION("can construct zero capacity book") {
        CHECK_NOTHROW(ptr.reset(new AnySizeBook(0)));
        // Curious little book - because it has 0 capacity, it is full and empty at the same time!
        REQUIRE(ptr->capacity == 0);
        REQUIRE(ptr->size<side::bid>() == 0);
        REQUIRE(ptr->full<side::bid>());
        REQUIRE(ptr->empty<side::bid>());
        REQUIRE(ptr->size<side::ask>() == 0);
        REQUIRE(ptr->full<side::ask>());
        REQUIRE(ptr->empty<side::ask>());
        REQUIRE(ptr->emplace_back<side::bid>() == npos);
        REQUIRE(ptr->emplace_back<side::ask>() == npos);

        // Using nothrow overload changes nothing, because the zero capacity is valid anyway
        CHECK_NOTHROW(ptr.reset(new AnySizeBook(0, nothrow)));
        REQUIRE(ptr->capacity == 0);
        REQUIRE(ptr->emplace_back<side::bid>() == npos);
        REQUIRE(ptr->emplace_back<side::ask>() == npos);
    }

    SECTION("can construct regular capacity book, no exceptions") {
        CHECK_NOTHROW(ptr.reset(new AnySizeBook(40)));
        REQUIRE(ptr->capacity == 40);
        REQUIRE(ptr->size<side::bid>() == 0);
        REQUIRE(ptr->size<side::ask>() == 0);

        CHECK_NOTHROW(ptr.reset(new AnySizeBook(126)));
        REQUIRE(ptr->capacity == 126);
        REQUIRE(ptr->size<side::bid>() == 0);
        REQUIRE(ptr->size<side::ask>() == 0);

        CHECK_NOTHROW(ptr.reset(new AnySizeBook(127)));
        REQUIRE(ptr->capacity == 127);
        REQUIRE(ptr->size<side::bid>() == 0);
        REQUIRE(ptr->size<side::ask>() == 0);
    }

    SECTION("invalid too-large capacity, exceptions thrown") {
        CHECK_THROWS_AS(ptr.reset(new AnySizeBook(128)), AnySizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new AnySizeBook(129)), AnySizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new AnySizeBook(130)), AnySizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new AnySizeBook(253)), AnySizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new AnySizeBook(254)), AnySizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new AnySizeBook(255)), AnySizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new AnySizeBook(256)), AnySizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new AnySizeBook(1000)), AnySizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new AnySizeBook((int) 1e9)), AnySizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new AnySizeBook(std::numeric_limits<int>::max())), AnySizeBook::bad_capacity);
    }

    SECTION("invalid negative capacity book, exceptions thrown") {
        CHECK_THROWS_AS(ptr.reset(new AnySizeBook(-1)), AnySizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new AnySizeBook(-126)), AnySizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new AnySizeBook(-127)), AnySizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new AnySizeBook(-128)), AnySizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new AnySizeBook(-129)), AnySizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new AnySizeBook(-130)), AnySizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new AnySizeBook(-253)), AnySizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new AnySizeBook(-254)), AnySizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new AnySizeBook(-255)), AnySizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new AnySizeBook((int) -1e9)), AnySizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new AnySizeBook(std::numeric_limits<int>::min())), AnySizeBook::bad_capacity);
    }

    SECTION("can construct regular capacity book, no exceptions and none allowed") {
        static_assert(std::is_same_v<AnySizeBook::nothrow_t, std::nothrow_t>);
        CHECK_NOTHROW(ptr.reset(new AnySizeBook(10, std::nothrow)));
        REQUIRE(ptr->capacity == 10);

        CHECK_NOTHROW(ptr.reset(new AnySizeBook(126, AnySizeBook::nothrow_t{})));
        REQUIRE(ptr->capacity == 126);

        CHECK_NOTHROW(ptr.reset(new AnySizeBook(127, nothrow)));
        REQUIRE(ptr->capacity == 127);
    }

    SECTION("invalid capacity book, reported zero capacity, no exceptions") {
        CHECK_NOTHROW(ptr.reset(new AnySizeBook(-1, nothrow)));
        // Same behaviour as 0 capacity book tested above
        CHECK(ptr->capacity == 0);
        REQUIRE(ptr->size<side::bid>() == 0);
        REQUIRE(ptr->full<side::bid>());
        REQUIRE(ptr->empty<side::bid>());
        REQUIRE(ptr->size<side::ask>() == 0);
        REQUIRE(ptr->full<side::ask>());
        REQUIRE(ptr->empty<side::ask>());
        REQUIRE(ptr->emplace_back<side::bid>() == npos);
        REQUIRE(ptr->emplace_back<side::ask>() == npos);

        CHECK_NOTHROW(ptr.reset(new AnySizeBook(128, nothrow)));
        CHECK(ptr->capacity == 0);
        REQUIRE(ptr->emplace_back<side::bid>() == npos);
        REQUIRE(ptr->emplace_back<side::ask>() == npos);

        CHECK_NOTHROW(ptr.reset(new AnySizeBook(-127, nothrow)));
        CHECK(ptr->capacity == 0);
        REQUIRE(ptr->emplace_back<side::bid>() == npos);
        REQUIRE(ptr->emplace_back<side::ask>() == npos);

        CHECK_NOTHROW(ptr.reset(new AnySizeBook((int) 1e9, nothrow)));
        CHECK(ptr->capacity == 0);
        REQUIRE(ptr->emplace_back<side::bid>() == npos);
        REQUIRE(ptr->emplace_back<side::ask>() == npos);

        CHECK_NOTHROW(ptr.reset(new AnySizeBook(std::numeric_limits<int>::min(), nothrow)));
        REQUIRE(ptr->emplace_back<side::bid>() == npos);
        REQUIRE(ptr->emplace_back<side::ask>() == npos);

        CHECK_NOTHROW(ptr.reset(new AnySizeBook(std::numeric_limits<int>::max(), nothrow)));
        CHECK(ptr->capacity == 0);
        REQUIRE(ptr->emplace_back<side::bid>() == npos);
        REQUIRE(ptr->emplace_back<side::ask>() == npos);
    }
}

TEST_CASE("AnySizeBook_find", "[book][find]") {
    using namespace market;
    constexpr auto npos = AnySizeBook::npos;

    // Q: why so many seemingly identical tests? A: there is a bit of branching inside
    //    find(), need to ensure we go through all possible execution paths.
    SECTION("find() in empty book") {
        AnySizeBook book {0};
        CHECK(book.find<side::ask>(130130) == npos);
        CHECK(book.find<side::bid>(130130) == npos);
    }

    SECTION("find() in book size=1") {
        AnySizeBook book {1};
        book.emplace_back<side::ask>(130130, 1);
        CHECK(book.find<side::ask>(130130) == 0);
        CHECK(book.find<side::ask>(130100) == npos);
        CHECK(book.find<side::ask>(130200) == npos);
    }

    SECTION("find() in book size=2") {
        AnySizeBook book {2};
        book.emplace_back<side::ask>(130130, 1);
        book.emplace_back<side::ask>(130132, 1);
        CHECK(book.find<side::ask>(130130) == 0);
        CHECK(book.find<side::ask>(130131) == npos);
        CHECK(book.find<side::ask>(130132) == 1);
        CHECK(book.find<side::ask>(130100) == npos);
        CHECK(book.find<side::ask>(130200) == npos);
    }

    SECTION("find() in book size=2, bid side") {
        AnySizeBook book {2};
        book.emplace_back<side::bid>(130132, 1);
        book.emplace_back<side::bid>(130130, 1);
        CHECK(book.find<side::bid>(130132) == 0);
        CHECK(book.find<side::bid>(130131) == npos);
        CHECK(book.find<side::bid>(130130) == 1);
        CHECK(book.find<side::bid>(130100) == npos);
        CHECK(book.find<side::bid>(130200) == npos);
    }

    SECTION("find() in book size=3") {
        AnySizeBook book {3};
        book.emplace_back<side::ask>(130130, 1);
        book.emplace_back<side::ask>(130132, 1);
        book.emplace_back<side::ask>(130134, 1);
        CHECK(book.find<side::ask>(130130) == 0);
        CHECK(book.find<side::ask>(130131) == npos);
        CHECK(book.find<side::ask>(130132) == 1);
        CHECK(book.find<side::ask>(130133) == npos);
        CHECK(book.find<side::ask>(130134) == 2);
        CHECK(book.find<side::ask>(130100) == npos);
        CHECK(book.find<side::ask>(130200) == npos);
    }

    SECTION("find() in book size=4") {
        AnySizeBook book {4};
        book.emplace_back<side::ask>(130130, 1);
        book.emplace_back<side::ask>(130132, 1);
        book.emplace_back<side::ask>(130134, 1);
        book.emplace_back<side::ask>(130136, 1);
        CHECK(book.find<side::ask>(130130) == 0);
        CHECK(book.find<side::ask>(130131) == npos);
        CHECK(book.find<side::ask>(130132) == 1);
        CHECK(book.find<side::ask>(130133) == npos);
        CHECK(book.find<side::ask>(130134) == 2);
        CHECK(book.find<side::ask>(130135) == npos);
        CHECK(book.find<side::ask>(130136) == 3);
        CHECK(book.find<side::ask>(130100) == npos);
        CHECK(book.find<side::ask>(130200) == npos);
    }

    SECTION("find() in book size=5") {
        AnySizeBook book {5};
        book.emplace_back<side::ask>(130130, 1);
        book.emplace_back<side::ask>(130132, 1);
        book.emplace_back<side::ask>(130134, 1);
        book.emplace_back<side::ask>(130136, 1);
        book.emplace_back<side::ask>(130138, 1);
        CHECK(book.find<side::ask>(130130) == 0);
        CHECK(book.find<side::ask>(130131) == npos);
        CHECK(book.find<side::ask>(130132) == 1);
        CHECK(book.find<side::ask>(130133) == npos);
        CHECK(book.find<side::ask>(130134) == 2);
        CHECK(book.find<side::ask>(130135) == npos);
        CHECK(book.find<side::ask>(130136) == 3);
        CHECK(book.find<side::ask>(130137) == npos);
        CHECK(book.find<side::ask>(130138) == 4);
        CHECK(book.find<side::ask>(130100) == npos);
        CHECK(book.find<side::ask>(130200) == npos);
    }

    SECTION("find() in book size=5, bid side") {
        AnySizeBook book {5};
        book.emplace_back<side::bid>(130138, 1);
        book.emplace_back<side::bid>(130136, 1);
        book.emplace_back<side::bid>(130134, 1);
        book.emplace_back<side::bid>(130132, 1);
        book.emplace_back<side::bid>(130130, 1);
        CHECK(book.find<side::bid>(130130) == 4);
        CHECK(book.find<side::bid>(130131) == npos);
        CHECK(book.find<side::bid>(130132) == 3);
        CHECK(book.find<side::bid>(130133) == npos);
        CHECK(book.find<side::bid>(130134) == 2);
        CHECK(book.find<side::bid>(130135) == npos);
        CHECK(book.find<side::bid>(130136) == 1);
        CHECK(book.find<side::bid>(130137) == npos);
        CHECK(book.find<side::bid>(130138) == 0);
        CHECK(book.find<side::bid>(130100) == npos);
        CHECK(book.find<side::bid>(130200) == npos);
    }
}

namespace {
    struct ConstLevel {
        const int ticks; // Regular assignment won't work here

        template <market::side Side>
        constexpr static bool compare(const ConstLevel& lh, const ConstLevel& rh) noexcept;

        constexpr static ConstLevel make(int i) {
            return ConstLevel{i};
        }
    };

    template <>
    constexpr bool ConstLevel::compare<market::side::bid>(const ConstLevel& lh, const ConstLevel& rh) noexcept {
        return lh.ticks > rh.ticks;
    }

    template <>
    constexpr bool ConstLevel::compare<market::side::ask>(const ConstLevel& lh, const ConstLevel& rh) noexcept {
        return lh.ticks < rh.ticks;
    }

    bool operator==(const ConstLevel& lh, const ConstLevel& rh) {
        return lh.ticks == rh.ticks;
    }

    std::ostream& operator<<(std::ostream& lh, const ConstLevel& rh) {
        return (lh << '{' << rh.ticks << '}');
    }
}

namespace {
    struct SmallConstBook : market::book<ConstLevel> {
        SmallConstBook() : book<ConstLevel>(levels, sides, freel, 2, 0, 0, nothrow)
        { } // No need to call accept() or reset(), free list populated by hand

        ConstLevel levels[4] = {};
        size_type sides[4] = {};
        size_type freel[4] = {0, 1, 2, 3};
    };
}

TEST_CASE("SmallConstBook_immutable", "[book][emplace_back][remove][sort][size][at]") {
    using namespace market;
    SmallConstBook book;
    REQUIRE(book.capacity == 2);

    SECTION("emplace_back() works with immutable data") {
        CHECK(book.emplace_back<side::ask>(120120) == 0);
        CHECK(book.size<side::ask>() == 1);
        CHECK(book.at<side::ask>(0) == ConstLevel{120120});

        CHECK(book.emplace_back<side::ask>(120118) == 1);
        CHECK(book.size<side::ask>() == 2);
        CHECK(book.at<side::ask>(0) == ConstLevel{120120});
        CHECK(book.at<side::ask>(1) == ConstLevel{120118});
        const auto* ptr = &book.at<side::ask>(1);

        CHECK(book.emplace_back<side::ask>(120114) == SmallConstBook::npos);
        CHECK(book.size<side::ask>() == 2);

        SECTION("remove() works on immutable data and frees space, which can be reused") {
            book.remove<side::ask>(0);
            CHECK(book.size<side::ask>() == 1);
            CHECK(book.at<side::ask>(0) == ConstLevel{120118});
            CHECK(ptr == &book.at<side::ask>(0));

            CHECK(book.emplace_back<side::ask>(120121) == 1);
            CHECK(book.size<side::ask>() == 2);
            CHECK(ptr == &book.at<side::ask>(0));
            CHECK(book.at<side::ask>(0) == ConstLevel{120118});
            CHECK(book.at<side::ask>(1) == ConstLevel{120121});

            CHECK(book.emplace_back<side::ask>(120114) == SmallConstBook::npos);
            CHECK(book.size<side::ask>() == 2);
        }

        SECTION("sort() works on immutable data") {
            book.sort<side::ask>();
            CHECK(book.size<side::ask>() == 2);
            CHECK(book.at<side::ask>(0) == ConstLevel{120118});
            CHECK(book.at<side::ask>(1) == ConstLevel{120120});

            CHECK(book.find<side::ask>(120118) == 0);
            CHECK(book.find<side::ask>(120120) == 1);
            CHECK(book.find<side::ask>(120100) == SmallBook::npos);
            CHECK(book.find<side::ask>(120131) == SmallBook::npos);
        }

        auto& level = book.at<side::ask>(0);
        static_assert(std::is_same_v<decltype(level.ticks), const int>);
    }
}

TEST_CASE("SmallConstBook_data", "[book][emplace_back][remove][sort][size][at]") {
    using namespace market;

    SmallConstBook book;
    REQUIRE(book.capacity == 2);
    CHECK(book.emplace_back<side::ask>(120120) == 0);
    CHECK(book.emplace_back<side::ask>(120118) == 1);

    SECTION("verify data organization") {
        // Note: this is NOT "white box testing". Some aspects of data organization are part of
        // the interface of market::book, because it does not own the memory for levels, sides
        // and free list. This enables users to create truly constexpr instances of book.
        REQUIRE(book.size<side::ask>() == 2);
        std::set<int> levels;

        // Bids are book.sides[0 .. 1], asks are book.sides[2 .. 3]. Here we inspect ask[0]
        // and ask[1], which should be stored in sides[2] and sides[3], i.e. capacity + index
        const auto a0 = book.sides[2];
        REQUIRE(a0 < 4);
        levels.insert(a0);
        CHECK(book.levels[a0] == ConstLevel{120120});

        const auto a1 = book.sides[3];
        REQUIRE(a1 < 4);
        levels.insert(a1);
        CHECK(book.levels[a1] == ConstLevel{120118});
        REQUIRE(levels.size() == 2);

        SECTION("verify data organization of free list") {
            // We should have 2 elements in the free list
            REQUIRE(book.freel[0] < 4);
            levels.insert(book.freel[0]);
            REQUIRE(book.freel[1] < 4);
            levels.insert(book.freel[1]);

            REQUIRE(levels.size() == 4);
        }
        // Do not read unused elements in data.levels, unused elements in data.sides
        // and remaining elements in free list.

        SECTION("verify data organization of full book") {
            CHECK(book.emplace_back<side::bid>(120115) == 0);
            CHECK(book.emplace_back<side::bid>(120116) == 1);

            // Here we inspect bid[0] and bid[1]
            const auto b0 = book.sides[0];
            REQUIRE(b0 < 4);
            levels.insert(b0);
            CHECK(book.levels[b0] == ConstLevel{120115});

            const auto b1 = book.sides[1];
            REQUIRE(b1 < 4);
            levels.insert(b1);
            CHECK(book.levels[b1] == ConstLevel{120116});
            REQUIRE(levels.size() == 4);
        }

        SECTION("verify sort impact on data organization") {
            book.sort<side::ask>();
            // The only thing that the sort had done was swapping these two indices
            CHECK(a0 == book.sides[3]);
            CHECK(a1 == book.sides[2]);
        }
    }
}

namespace {
    struct Empty : market::book<Level> {
        constexpr Empty() : book<Level>(nullptr, nullptr, nullptr, 0, 0, 0, nothrow)
        { }
    };
}

TEST_CASE("Empty_constexpr", "[book][capacity][size][full][empty][constexpr]") {
    using namespace market;
    SECTION("empty constexpr instance") {
        constexpr Empty book;
        REQUIRE(book.capacity == 0);
        REQUIRE(book.size<side::bid>() == 0);
        REQUIRE(book.empty<side::bid>());
        REQUIRE(book.full<side::bid>());
        REQUIRE(book.size<side::ask>() == 0);
        REQUIRE(book.empty<side::ask>());
        REQUIRE(book.full<side::ask>());
        REQUIRE(book.find<side::ask>(130130) == Empty::npos);
    }
}

namespace {
    struct OneLevel : market::book<ConstLevel> {
        constexpr OneLevel() : book<ConstLevel>(levels, sides, 1, 1, 1, nothrow)
        { }

        constexpr static ConstLevel levels[2] = {{130120}, {130125}};
        constexpr static size_type sides[2] = {0, 1};
    };
}

TEST_CASE("OneLevel_constexpr", "[book][capacity][size][full][empty][at][constexpr]") {
    // This is not very practical, but cool. Could be useful in unit tests
    using namespace market;
    SECTION("empty constexpr instance") {
        constexpr OneLevel book;
        REQUIRE(book.capacity == 1);
        REQUIRE(book.size<side::bid>() == 1);
        REQUIRE(not book.empty<side::bid>());
        REQUIRE(book.full<side::bid>());
        CHECK(book.at<side::bid>(0) == ConstLevel{130120});
        REQUIRE(book.size<side::ask>() == 1);
        REQUIRE(not book.empty<side::ask>());
        REQUIRE(book.full<side::ask>());
        CHECK(book.at<side::ask>(0) == ConstLevel{130125});
        CHECK(book.find<side::bid>(130120) == 0);
        CHECK(book.find<side::ask>(130125) == 0);
    }
}

namespace {
    struct OneSided : market::book<ConstLevel> {
        constexpr OneSided() : book<ConstLevel>(levels, sides, 1, 1, 0, nothrow)
        { }

        constexpr static ConstLevel levels[2] = {{130120}};
        constexpr static size_type sides[2] = {0};
    };
}

TEST_CASE("OneSided_constexpr", "[book][capacity][size][full][empty][at][constexpr]") {
    // This is not very practical, but cool. Could be useful in unit tests
    using namespace market;
    SECTION("empty constexpr instance") {
        constexpr OneSided book;
        REQUIRE(book.capacity == 1);
        REQUIRE(book.size<side::bid>() == 1);
        REQUIRE(not book.empty<side::bid>());
        REQUIRE(book.full<side::bid>());
        CHECK(book.at<side::bid>(0) == ConstLevel{130120});
        // nothing on this side
        REQUIRE(book.size<side::ask>() == 0);
        REQUIRE(book.empty<side::ask>());
        REQUIRE(not book.full<side::ask>());
        CHECK(book.find<side::bid>(130120) == 0);
        CHECK(book.find<side::ask>(130125) == OneSided::npos);
    }
}
