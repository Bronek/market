// Copyright (c) 2018 Bronislaw (Bronek) Kozicki
//
// Distributed under the MIT License. See accompanying file LICENSE
// or copy at https://opensource.org/licenses/MIT

#include <market/book.hpp>

#include <catch2/catch.hpp>

namespace {
    struct Level {
        int ticks; int size;
    };

    bool operator==(const Level& lh, const Level& rh) {
        return lh.ticks == rh.ticks && lh.size == rh.size;
    }

    std::ostream& operator<<(std::ostream& lh, const Level& rh) {
        return (lh << '{' << rh.ticks << ',' << rh.size << '}');
    }

    struct SmallBook : market::book<Level> {
        SmallBook() : book<Level>(data) {
            reset();
        }

        book::data<4> data;
    };
}

TEST_CASE("SmallBook_full", "[book][empty][full][data][capacity][push_back][emplace_back][remove][size][at]") {
    // Check all functions when book is full with "normal" liquidity levels
    using namespace market;

    SmallBook book;
    const auto& p4 = book.data;
    REQUIRE(p4.capacity == 4);
    const int back = p4.capacity * 2 - 1;
    REQUIRE(back == 7);
    REQUIRE(sizeof(p4.levels) / sizeof(p4.levels[0]) == back + 1);
    REQUIRE(sizeof(p4.sides) / sizeof(p4.sides[0]) == back + 1);
    REQUIRE(sizeof(p4.freel) / sizeof(p4.freel[0]) == back + 1);

    REQUIRE(book.capacity == 4);
    REQUIRE(book.size<side::ask>() == 0);
    CHECK(book.empty<side::ask>());
    CHECK(not book.full<side::ask>());
    REQUIRE(book.size<side::bid>() == 0);
    CHECK(book.empty<side::bid>());
    CHECK(not book.full<side::bid>());

    // Add elements to book - different method for each element
    REQUIRE(book.emplace_back<side::ask>(120125, 500) == 0);
    REQUIRE(book.size<side::ask>() == 1);
    CHECK(not book.empty<side::ask>());
    CHECK(not book.full<side::ask>());
    CHECK(book.at<side::ask>(0) == Level{120125, 500});

    // Remove just added element, then add again
    book.remove<side::ask>(0);
    REQUIRE(book.size<side::ask>() == 0);
    CHECK(book.empty<side::ask>());
    CHECK(not book.full<side::ask>());

    REQUIRE(book.emplace_back<side::ask>(120121, 300) == 0);
    REQUIRE(book.size<side::ask>() == 1);
    CHECK(not book.empty<side::ask>());
    CHECK(not book.full<side::ask>());
    // Verify the newly inserted levels are stored inside p4.levels
    const auto *ptr0a = &book.at<side::ask>(0);
    REQUIRE(ptr0a >= &p4.levels[0]);
    REQUIRE(ptr0a <= &p4.levels[back]);
    CHECK(*ptr0a == Level{120121, 300});

    REQUIRE(book.push_back<side::ask>(Level{120122, 300}) == 1);
    REQUIRE(book.size<side::ask>() == 2);
    CHECK(not book.empty<side::ask>());
    CHECK(not book.full<side::ask>());
    const auto *ptr1a = &book.at<side::ask>(1);
    REQUIRE(ptr1a >= &p4.levels[0]);
    REQUIRE(ptr1a <= &p4.levels[back]);
    CHECK(*ptr1a == Level{120122, 300});
    REQUIRE(ptr0a != ptr1a);

    REQUIRE(book.emplace_back<side::ask>(Level{120123, 300}) == 2);
    REQUIRE(book.size<side::ask>() == 3);
    CHECK(not book.empty<side::ask>());
    CHECK(not book.full<side::ask>());
    const auto *ptr2a = &book.at<side::ask>(2);
    REQUIRE(ptr2a >= &p4.levels[0]);
    REQUIRE(ptr2a <= &p4.levels[back]);
    CHECK(*ptr2a == Level{120123, 300});
    REQUIRE(ptr0a != ptr2a);
    REQUIRE(ptr1a != ptr2a);

    auto&& tmp1 = Level{120126, 600};
    REQUIRE(book.push_back<side::ask>(tmp1) == 3);
    REQUIRE(book.size<side::ask>() == 4);
    CHECK(not book.empty<side::ask>());
    CHECK(book.full<side::ask>());
    CHECK(book.at<side::ask>(3) == Level{120126, 600});
    CHECK(book.emplace_back<side::ask>() == book.npos);

    // Remove just added element, verify pointers still valid
    book.remove<side::ask>(3);
    REQUIRE(book.size<side::ask>() == 3);
    CHECK(not book.empty<side::ask>());
    CHECK(not book.full<side::ask>());
    REQUIRE(ptr0a == &book.at<side::ask>(0));
    CHECK(*ptr0a == Level{120121, 300});
    REQUIRE(ptr1a == &book.at<side::ask>(1));
    CHECK(*ptr1a == Level{120122, 300});
    REQUIRE(ptr2a == &book.at<side::ask>(2));
    CHECK(*ptr2a == Level{120123, 300});

    tmp1 = Level{120124, 300};
    REQUIRE(book.emplace_back<side::ask>(tmp1) == 3);
    REQUIRE(book.size<side::ask>() == 4);
    CHECK(not book.empty<side::ask>());
    CHECK(book.full<side::ask>());
    const auto *ptr3a = &book.at<side::ask>(3);
    REQUIRE(ptr3a >= &p4.levels[0]);
    REQUIRE(ptr3a <= &p4.levels[back]);
    CHECK(*ptr3a == Level{120124, 300});
    REQUIRE(ptr0a != ptr3a);
    REQUIRE(ptr1a != ptr3a);
    REQUIRE(ptr2a != ptr3a);
    CHECK(book.push_back<side::ask>(std::move(tmp1)) == book.npos);

    // Add elements on bid side
    auto&& tmp2 = Level{120120, 300};
    REQUIRE(book.emplace_back<side::bid>(tmp2) == 0);
    REQUIRE(book.size<side::bid>() == 1);
    CHECK(not book.empty<side::bid>());
    CHECK(not book.full<side::bid>());
    const auto *ptr0b = &book.at<side::bid>(0);
    REQUIRE(ptr0b >= &p4.levels[0]);
    REQUIRE(ptr0b <= &p4.levels[back]);
    CHECK(*ptr0b == Level{120120, 300});
    REQUIRE(ptr0a != ptr0b);
    REQUIRE(ptr1a != ptr0b);
    REQUIRE(ptr2a != ptr0b);
    REQUIRE(ptr3a != ptr0b);

    auto&& tmp3 = Level{120115, 700};
    REQUIRE(book.emplace_back<side::bid>(std::move(tmp3)) == 1);
    REQUIRE(book.size<side::bid>() == 2);
    CHECK(not book.empty<side::bid>());
    CHECK(not book.full<side::bid>());
    const auto *ptr1b = &book.at<side::bid>(1);
    REQUIRE(ptr1b >= &p4.levels[0]);
    REQUIRE(ptr1b <= &p4.levels[back]);
    CHECK(*ptr1b == Level{120115, 700});
    REQUIRE(ptr0a != ptr1b);
    REQUIRE(ptr1a != ptr1b);
    REQUIRE(ptr2a != ptr1b);
    REQUIRE(ptr3a != ptr1b);
    REQUIRE(ptr0b != ptr1b);

    const auto& tmp4 = Level{120118, 300};
    REQUIRE(book.push_back<side::bid>(tmp4) == 2);
    REQUIRE(book.size<side::bid>() == 3);
    CHECK(not book.empty<side::bid>());
    CHECK(not book.full<side::bid>());
    const auto *ptr2b = &book.at<side::bid>(2);
    REQUIRE(ptr2b >= &p4.levels[0]);
    REQUIRE(ptr2b <= &p4.levels[back]);
    CHECK(*ptr2b == Level{120118, 300});
    REQUIRE(ptr0a != ptr2b);
    REQUIRE(ptr1a != ptr2b);
    REQUIRE(ptr2a != ptr2b);
    REQUIRE(ptr3a != ptr2b);
    REQUIRE(ptr0b != ptr2b);
    REQUIRE(ptr1b != ptr2b);

    // Remove preceding element
    book.remove<side::bid>(1);
    REQUIRE(book.size<side::bid>() == 2);
    CHECK(not book.empty<side::bid>());
    CHECK(not book.full<side::bid>());
    REQUIRE(ptr0a == &book.at<side::ask>(0));
    CHECK(*ptr0a == Level{120121, 300});
    REQUIRE(ptr1a == &book.at<side::ask>(1));
    CHECK(*ptr1a == Level{120122, 300});
    REQUIRE(ptr2a == &book.at<side::ask>(2));
    CHECK(*ptr2a == Level{120123, 300});
    REQUIRE(ptr3a == &book.at<side::ask>(3));
    CHECK(*ptr3a == Level{120124, 300});
    REQUIRE(ptr0b == &book.at<side::bid>(0));
    CHECK(*ptr0b == Level{120120, 300});
    REQUIRE(ptr2b == &book.at<side::bid>(1)); // Shifted from position 2, due to remove(1) above
    CHECK(*ptr2b == Level{120118, 300});

    REQUIRE(book.emplace_back<side::bid>(Level{120119, 300}) == 2);
    REQUIRE(book.size<side::bid>() == 3);
    CHECK(not book.empty<side::bid>());
    CHECK(not book.full<side::bid>());
    ptr1b = &book.at<side::bid>(2);
    REQUIRE(ptr1b >= &p4.levels[0]);
    REQUIRE(ptr1b <= &p4.levels[back]);
    CHECK(*ptr1b == Level{120119, 300});
    REQUIRE(ptr0a != ptr1b);
    REQUIRE(ptr1a != ptr1b);
    REQUIRE(ptr2a != ptr1b);
    REQUIRE(ptr3a != ptr1b);
    REQUIRE(ptr0b != ptr1b);
    REQUIRE(ptr2b != ptr1b);

    const Level tmp5 {120117, 300};
    REQUIRE(book.push_back<side::bid>(tmp5) == 3);
    REQUIRE(book.size<side::bid>() == 4);
    CHECK(not book.empty<side::bid>());
    CHECK(book.full<side::bid>());
    const auto *ptr3b = &book.at<side::bid>(3);
    REQUIRE(ptr3b >= &p4.levels[0]);
    REQUIRE(ptr3b <= &p4.levels[back]);
    CHECK(*ptr3b == Level{120117, 300});
    REQUIRE(ptr0a != ptr3b);
    REQUIRE(ptr1a != ptr3b);
    REQUIRE(ptr2a != ptr3b);
    REQUIRE(ptr3a != ptr3b);
    REQUIRE(ptr0b != ptr3b);
    REQUIRE(ptr1b != ptr3b);
    REQUIRE(ptr2b != ptr3b);
    Level tmp6 {120116, 300};
    CHECK(book.emplace_back<side::bid>(std::move(tmp6)) == book.npos);

    CHECK(*ptr0a == Level{120121, 300});
    CHECK(*ptr1a == Level{120122, 300});
    CHECK(*ptr2a == Level{120123, 300});
    CHECK(*ptr3a == Level{120124, 300});
    CHECK(*ptr0b == Level{120120, 300});
    CHECK(*ptr1b == Level{120119, 300});
    CHECK(*ptr2b == Level{120118, 300});
    CHECK(*ptr3b == Level{120117, 300});
}

TEST_CASE("SmallBook_irregular", "[book][emplace_back][push_back][remove][size][full][empty][at][Unordered][Duplicate]") {
    // Check book populated with irregular (duplicate or unordered) liquidity
    using namespace market;
    SmallBook book;
    CHECK(book.capacity == 4);

    REQUIRE(book.size<side::bid>() == 0);
    REQUIRE(book.emplace_back<side::bid>(120120, 200) == 0);
    REQUIRE(book.emplace_back<side::bid>(120120, 100) == 1);
    REQUIRE(book.emplace_back<side::bid>(120120, 400) == 2);
    REQUIRE(book.emplace_back<side::bid>(120120, 300) == 3);
    CHECK(book.full<side::bid>());

    REQUIRE(book.emplace_back<side::bid>(120120, 100) == SmallBook::npos);
    REQUIRE(book.size<side::bid>() == 4);

    CHECK(book.empty<side::ask>());
    REQUIRE(book.push_back<side::ask>(Level{120122, 200}) == 0);
    REQUIRE(book.push_back<side::ask>(Level{120118, 600}) == 1);
    REQUIRE(book.push_back<side::ask>(Level{120118, 300}) == 2);
    REQUIRE(book.push_back<side::ask>(Level{120122, 400}) == 3);
    REQUIRE(book.size<side::ask>() == 4);

    REQUIRE(book.emplace_back<side::ask>(120122, 100) == SmallBook::npos);
    CHECK(book.full<side::ask>());

    CHECK(book.at<side::bid>(0) == Level{120120, 200});
    CHECK(book.at<side::bid>(1) == Level{120120, 100});
    CHECK(book.at<side::bid>(2) == Level{120120, 400});
    CHECK(book.at<side::bid>(3) == Level{120120, 300});
    CHECK(book.at<side::ask>(0) == Level{120122, 200});
    CHECK(book.at<side::ask>(1) == Level{120118, 600});
    CHECK(book.at<side::ask>(2) == Level{120118, 300});
    CHECK(book.at<side::ask>(3) == Level{120122, 400});

    // Test removes
    book.remove<side::bid>(3);
    book.remove<side::bid>(2);
    book.remove<side::bid>(0);
    // The level below was at index 1, but shifted to 0 due to above remove(0)
    REQUIRE(book.size<side::bid>() == 1);
    CHECK(book.at<side::bid>(0) == Level{120120, 100});
    book.remove<side::bid>(0);
    REQUIRE(book.empty<side::bid>());

    book.remove<side::ask>(0);
    REQUIRE(book.size<side::ask>() == 3);
    CHECK(book.at<side::ask>(0) == Level{120118, 600});
    book.remove<side::ask>(0);
    REQUIRE(book.size<side::ask>() == 2);
    CHECK(book.at<side::ask>(0) == Level{120118, 300});
    book.remove<side::ask>(1);
    REQUIRE(book.size<side::ask>() == 1);
    CHECK(book.at<side::ask>(0) == Level{120118, 300});
    book.remove<side::ask>(0);
    REQUIRE(book.size<side::ask>() == 0);
    REQUIRE(book.empty<side::ask>());
}

namespace {
    struct ConstLevel {
        const int ticks; const int size; // Regular assignment won't work here
    };

    bool operator==(const ConstLevel& lh, const ConstLevel& rh) {
        return lh.ticks == rh.ticks && lh.size == rh.size;
    }

    std::ostream& operator<<(std::ostream& lh, const ConstLevel& rh) {
        return (lh << '{' << rh.ticks << ',' << rh.size << '}');
    }

    struct SmallConstBook : market::book<ConstLevel> {
        SmallConstBook() : book<ConstLevel>(levels, sides, freel, 2, nothrow, 0, 0)
        { } // No need to call accept() or reset(), free list populated by hand

        ConstLevel levels[4] = {};
        size_type sides[4] = {};
        size_type freel[4] = {0, 1, 2, 3};
    };
}

TEST_CASE("SmallConstBook", "[book][emplace_back][size][full][empty][at][Immutable]") {
    // Test that emplace_back and remove work with immutable levels
    using namespace market;
    SmallConstBook book;
    CHECK(book.capacity == 2);

    REQUIRE(book.emplace_back<side::bid>(120120, 200) == 0);
    REQUIRE(book.size<side::bid>() == 1);
    CHECK(book.at<side::bid>(0) == ConstLevel{120120, 200});

    REQUIRE(book.emplace_back<side::bid>(120118, 500) == 1);
    REQUIRE(book.size<side::bid>() == 2);
    CHECK(book.at<side::bid>(0) == ConstLevel{120120, 200});
    CHECK(book.at<side::bid>(1) == ConstLevel{120118, 500});
    const auto* ptr = &book.at<side::bid>(1);
    CHECK(not book.empty<side::bid>());
    CHECK(book.full<side::bid>());

    CHECK(book.emplace_back<side::bid>(120114, 800) == SmallConstBook::npos);
    REQUIRE(book.size<side::bid>() == 2);

    book.remove<side::bid>(0);
    REQUIRE(book.size<side::bid>() == 1);
    CHECK(book.at<side::bid>(0) == ConstLevel{120118, 500});
    CHECK(not book.empty<side::bid>());
    CHECK(not book.full<side::bid>());
    CHECK(ptr == &book.at<side::bid>(0));

    REQUIRE(book.emplace_back<side::bid>(120120, 200) == 1);
    REQUIRE(book.size<side::bid>() == 2);
    CHECK(ptr == &book.at<side::bid>(0));

    CHECK(book.emplace_back<side::bid>(120114, 800) == SmallConstBook::npos);
    REQUIRE(book.size<side::bid>() == 2);
    CHECK(book.at<side::bid>(0) == ConstLevel{120118, 500});
    CHECK(book.at<side::bid>(1) == ConstLevel{120120, 200});
    CHECK(not book.empty<side::bid>());
    CHECK(book.full<side::bid>());

    REQUIRE(book.emplace_back<side::ask>(120120, 400) == 0);
    REQUIRE(book.emplace_back<side::ask>(120120, 500) == 1);
    REQUIRE(book.emplace_back<side::ask>(120120, 400) == SmallConstBook::npos);
    REQUIRE(book.size<side::ask>() == 2);
    CHECK(book.at<side::ask>(0) == ConstLevel{120120, 400});
    CHECK(book.at<side::ask>(1) == ConstLevel{120120, 500});
    CHECK(not book.empty<side::ask>());
    CHECK(book.full<side::ask>());

    auto& level = book.at<side::bid>(0);
    static_assert(std::is_same_v<decltype(level.ticks), const int>);
    static_assert(std::is_same_v<decltype(level.size), const int>);
}

namespace {
    struct DummyLevel {};

    struct PretendSizeBook : market::book<DummyLevel> {
        // Normally these two will not be exposed
        using book::nothrow_t;
        using book::nothrow;

        DummyLevel levels[254] = {};
        uint8_t sides[254] = {};
        uint8_t freel[254] = {};

        template <typename ... Args>
        PretendSizeBook(int i, Args&& ... t)
            : book(levels, sides, freel, i, std::forward<Args>(t)...) {
            this->accept();
        }
    };
}

TEST_CASE("PretendSizeBook", "[book][construction][bad_capacity][nothrow][Exceptions][ZeroCapacity][Validation]") {
    using namespace market;

    std::unique_ptr<PretendSizeBook> ptr = nullptr;
    SECTION("zero capacity book, no exceptions") {
        CHECK_NOTHROW(ptr.reset(new PretendSizeBook(0)));
        // Curious little container - because it has 0 capacity, it is full and empty at the same time!
        REQUIRE(ptr->capacity == 0);
        REQUIRE(ptr->size<side::bid>() == 0);
        REQUIRE(ptr->full<side::bid>());
        REQUIRE(ptr->empty<side::bid>());
        REQUIRE(ptr->size<side::ask>() == 0);
        REQUIRE(ptr->full<side::ask>());
        REQUIRE(ptr->empty<side::ask>());
        REQUIRE(ptr->emplace_back<side::bid>() == PretendSizeBook::npos);
        REQUIRE(ptr->emplace_back<side::ask>() == PretendSizeBook::npos);

        CHECK_NOTHROW(ptr.reset(new PretendSizeBook(0, PretendSizeBook::nothrow)));
        REQUIRE(ptr->capacity == 0);
        REQUIRE(ptr->size<side::bid>() == 0);
        REQUIRE(ptr->full<side::bid>());
        REQUIRE(ptr->empty<side::bid>());
        REQUIRE(ptr->size<side::ask>() == 0);
        REQUIRE(ptr->full<side::ask>());
        REQUIRE(ptr->empty<side::ask>());
        REQUIRE(ptr->emplace_back<side::bid>() == PretendSizeBook::npos);
        REQUIRE(ptr->emplace_back<side::ask>() == PretendSizeBook::npos);
    }

    SECTION("regular capacity book, no exceptions") {
        CHECK_NOTHROW(ptr.reset(new PretendSizeBook(40)));
        REQUIRE(ptr->capacity == 40);
        REQUIRE(ptr->size<side::bid>() == 0);
        REQUIRE(ptr->size<side::ask>() == 0);

        CHECK_NOTHROW(ptr.reset(new PretendSizeBook(126)));
        REQUIRE(ptr->capacity == 126);
        REQUIRE(ptr->size<side::bid>() == 0);
        REQUIRE(ptr->size<side::ask>() == 0);

        CHECK_NOTHROW(ptr.reset(new PretendSizeBook(127)));
        REQUIRE(ptr->capacity == 127);
        REQUIRE(ptr->size<side::bid>() == 0);
        REQUIRE(ptr->size<side::ask>() == 0);
    }

    SECTION("invalid too-large capacity, exceptions thrown") {
        CHECK_THROWS_AS(ptr.reset(new PretendSizeBook(128)), PretendSizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new PretendSizeBook(129)), PretendSizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new PretendSizeBook(130)), PretendSizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new PretendSizeBook(253)), PretendSizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new PretendSizeBook(254)), PretendSizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new PretendSizeBook(255)), PretendSizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new PretendSizeBook(256)), PretendSizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new PretendSizeBook(1000)), PretendSizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new PretendSizeBook((int) 1e9)), PretendSizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new PretendSizeBook(std::numeric_limits<int>::max())), PretendSizeBook::bad_capacity);
    }

    SECTION("invalid negative capacity book, exceptions thrown") {
        CHECK_THROWS_AS(ptr.reset(new PretendSizeBook(-1)), PretendSizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new PretendSizeBook(-126)), PretendSizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new PretendSizeBook(-127)), PretendSizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new PretendSizeBook(-128)), PretendSizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new PretendSizeBook(-129)), PretendSizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new PretendSizeBook(-130)), PretendSizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new PretendSizeBook(-253)), PretendSizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new PretendSizeBook(-254)), PretendSizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new PretendSizeBook(-255)), PretendSizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new PretendSizeBook((int) -1e9)), PretendSizeBook::bad_capacity);
        CHECK_THROWS_AS(ptr.reset(new PretendSizeBook(std::numeric_limits<int>::min())), PretendSizeBook::bad_capacity);
    }

    SECTION("regular capacity book, no exceptions and none allowed") {
        static_assert(std::is_same_v<PretendSizeBook::nothrow_t, std::nothrow_t>);
        CHECK_NOTHROW(ptr.reset(new PretendSizeBook(10, std::nothrow)));
        REQUIRE(ptr->capacity == 10);

        CHECK_NOTHROW(ptr.reset(new PretendSizeBook(126, PretendSizeBook::nothrow_t{})));
        REQUIRE(ptr->capacity == 126);

        CHECK_NOTHROW(ptr.reset(new PretendSizeBook(127, PretendSizeBook::nothrow)));
        REQUIRE(ptr->capacity == 127);
    }

    // We do not want these to throw, even though the size passed to the book constructor is invalid
    SECTION("invalid capacity book, reported zero capacity, no exceptions") {
        CHECK_NOTHROW(ptr.reset(new PretendSizeBook(-1, PretendSizeBook::nothrow)));
        CHECK(ptr->capacity == 0);
        REQUIRE(ptr->size<side::bid>() == 0);
        REQUIRE(ptr->full<side::bid>());
        REQUIRE(ptr->empty<side::bid>());
        REQUIRE(ptr->size<side::ask>() == 0);
        REQUIRE(ptr->full<side::ask>());
        REQUIRE(ptr->empty<side::ask>());
        REQUIRE(ptr->emplace_back<side::bid>() == PretendSizeBook::npos);
        REQUIRE(ptr->emplace_back<side::ask>() == PretendSizeBook::npos);

        CHECK_NOTHROW(ptr.reset(new PretendSizeBook(128, PretendSizeBook::nothrow)));
        CHECK(ptr->capacity == 0);
        REQUIRE(ptr->size<side::bid>() == 0);
        REQUIRE(ptr->full<side::bid>());
        REQUIRE(ptr->empty<side::bid>());
        REQUIRE(ptr->size<side::ask>() == 0);
        REQUIRE(ptr->full<side::ask>());
        REQUIRE(ptr->empty<side::ask>());
        REQUIRE(ptr->emplace_back<side::bid>() == PretendSizeBook::npos);
        REQUIRE(ptr->emplace_back<side::ask>() == PretendSizeBook::npos);

        CHECK_NOTHROW(ptr.reset(new PretendSizeBook(-127, PretendSizeBook::nothrow)));
        CHECK(ptr->capacity == 0);
        REQUIRE(ptr->size<side::bid>() == 0);
        REQUIRE(ptr->full<side::bid>());
        REQUIRE(ptr->empty<side::bid>());
        REQUIRE(ptr->size<side::ask>() == 0);
        REQUIRE(ptr->full<side::ask>());
        REQUIRE(ptr->empty<side::ask>());
        REQUIRE(ptr->emplace_back<side::bid>() == PretendSizeBook::npos);
        REQUIRE(ptr->emplace_back<side::ask>() == PretendSizeBook::npos);

        CHECK_NOTHROW(ptr.reset(new PretendSizeBook((int) 1e9, PretendSizeBook::nothrow)));
        CHECK(ptr->capacity == 0);
        REQUIRE(ptr->size<side::bid>() == 0);
        REQUIRE(ptr->full<side::bid>());
        REQUIRE(ptr->empty<side::bid>());
        REQUIRE(ptr->size<side::ask>() == 0);
        REQUIRE(ptr->full<side::ask>());
        REQUIRE(ptr->empty<side::ask>());
        REQUIRE(ptr->emplace_back<side::bid>() == PretendSizeBook::npos);
        REQUIRE(ptr->emplace_back<side::ask>() == PretendSizeBook::npos);

        CHECK_NOTHROW(ptr.reset(new PretendSizeBook(std::numeric_limits<int>::min(), PretendSizeBook::nothrow)));
        CHECK(ptr->capacity == 0);
        REQUIRE(ptr->size<side::bid>() == 0);
        REQUIRE(ptr->full<side::bid>());
        REQUIRE(ptr->empty<side::bid>());
        REQUIRE(ptr->size<side::ask>() == 0);
        REQUIRE(ptr->full<side::ask>());
        REQUIRE(ptr->empty<side::ask>());
        REQUIRE(ptr->emplace_back<side::bid>() == PretendSizeBook::npos);
        REQUIRE(ptr->emplace_back<side::ask>() == PretendSizeBook::npos);

        CHECK_NOTHROW(ptr.reset(new PretendSizeBook(std::numeric_limits<int>::max(), PretendSizeBook::nothrow)));
        CHECK(ptr->capacity == 0);
        REQUIRE(ptr->size<side::bid>() == 0);
        REQUIRE(ptr->full<side::bid>());
        REQUIRE(ptr->empty<side::bid>());
        REQUIRE(ptr->size<side::ask>() == 0);
        REQUIRE(ptr->full<side::ask>());
        REQUIRE(ptr->empty<side::ask>());
        REQUIRE(ptr->emplace_back<side::bid>() == PretendSizeBook::npos);
        REQUIRE(ptr->emplace_back<side::ask>() == PretendSizeBook::npos);
    }
}

namespace {
    template <typename Level>
    struct PayloadBook : market::book<Level> {
        template <int Size>
        using data = typename market::book<Level>::template data<Size>;

        template <int Size, typename ... Args>
        explicit constexpr PayloadBook(data<Size>& p, Args&& ... t)
                : market::book<Level>(p, std::forward<Args>(t)...) {
            this->accept();
        }
    };
}

TEST_CASE("PayloadBook_stack", "[book][construction][accept][book_data][stack]") {
    using namespace market;

    PayloadBook<ConstLevel>::data<4> p4 = {};
    // Calculate index of last element in p4.levels from capacity
    REQUIRE(p4.capacity == 4);
    const int back = p4.capacity * 2 - 1;
    REQUIRE(back == 7);
    REQUIRE(sizeof(p4.levels) / sizeof(p4.levels[0]) == back + 1);
    REQUIRE(sizeof(p4.sides) / sizeof(p4.sides[0]) == back + 1);
    REQUIRE(sizeof(p4.freel) / sizeof(p4.freel[0]) == back + 1);

    SECTION("regular constructor") {
        const ConstLevel* ptr = nullptr;
        {
            auto&& b4 = PayloadBook<ConstLevel> {p4};
            REQUIRE(b4.capacity == 4);
            CHECK(b4.empty<side::bid>());
            REQUIRE(b4.emplace_back<side::bid>(120120, 200) == 0);
            REQUIRE(b4.size<side::bid>() == 1);
            // Verify the newly inserted level is stored (somewhere) inside p4.levels
            ptr = &b4.at<side::bid>(0);
            REQUIRE(ptr >= &p4.levels[0]);
            REQUIRE(ptr <= &p4.levels[back]);
        }

        // Verify data is persisted and accessible in another instance of PayloadBook
        {
            PayloadBook<ConstLevel> b5{p4, 1, 0};
            REQUIRE(b5.capacity == 4);
            REQUIRE(b5.size<side::bid>() == 1);
            CHECK(b5.empty<side::ask>());
            REQUIRE(ptr == &b5.at<side::bid>(0));
            CHECK(b5.at<side::bid>(0) == ConstLevel{120120, 200});
        }

        // Reset all levels, omitting size parameters in constructor
        {
            PayloadBook<ConstLevel> b6{p4};
            REQUIRE(b6.capacity == 4);
            CHECK(b6.empty<side::bid>());
            CHECK(b6.empty<side::ask>());
        }
    }

    // Use scope to destroy b4 and check that p4.levels remain in place, as populated by b4
    SECTION("book::data lifetime independent of book") {
        {
            PayloadBook<ConstLevel> b4(p4);
            REQUIRE(b4.capacity == 4);
            CHECK(b4.empty<side::ask>());
            CHECK(not b4.full<side::ask>());

            // Add elements on ask side
            REQUIRE(b4.emplace_back<side::ask>(120121, 121) == 0);
            REQUIRE(b4.size<side::ask>() == 1);
            CHECK(not b4.empty<side::ask>());
            CHECK(not b4.full<side::ask>());
            // Verify the newly inserted level is stored inside p4.levels
            const auto *ptr0a = &b4.at<side::ask>(0);
            REQUIRE(ptr0a >= &p4.levels[0]);
            REQUIRE(ptr0a <= &p4.levels[back]);
            CHECK(*ptr0a == ConstLevel{120121, 121});

            REQUIRE(b4.emplace_back<side::ask>(120122, 122) == 1);
            REQUIRE(b4.emplace_back<side::ask>(120123, 123) == 2);
            REQUIRE(b4.emplace_back<side::ask>(120124, 124) == 3);
            CHECK(b4.full<side::ask>());
            CHECK(b4.empty<side::bid>());
            REQUIRE(b4.emplace_back<side::ask>() == b4.npos);

            REQUIRE(b4.emplace_back<side::bid>(120120, 120) == 0);
            REQUIRE(b4.emplace_back<side::bid>(120120, 120) == 1);
            REQUIRE(b4.emplace_back<side::bid>(120119, 119) == 2);
            REQUIRE(b4.emplace_back<side::bid>(120119, 119) == 3);
            CHECK(b4.full<side::ask>());
            CHECK(b4.full<side::bid>());
            REQUIRE(b4.emplace_back<side::bid>() == b4.npos);
        }

        for (auto &l : p4.levels) {
            CHECK(l.ticks >= 120119);
            CHECK(l.ticks <= 120124);
            CHECK(l.size == l.ticks - 120000);
        }
    }
}

TEST_CASE("PayloadBook_heap", "[book][construction][book_data][heap]") {
    using namespace market;

    SECTION("regular capacity book, compile-time validation of capacity") {
        std::unique_ptr<PayloadBook<Level>> ptr1 = nullptr;
        PayloadBook<Level>::data<20> p1 = {};
        CHECK_NOTHROW(ptr1.reset(new PayloadBook<Level>(p1)));
        REQUIRE(ptr1->capacity == 20);
        REQUIRE(ptr1->capacity == p1.capacity);
        REQUIRE(ptr1->size<side::bid>() == 0);
        REQUIRE(ptr1->size<side::ask>() == 0);

        std::unique_ptr<PayloadBook<DummyLevel>> ptr3 = nullptr;
        PayloadBook<DummyLevel>::data<1> p3 = {};
        CHECK_NOTHROW(ptr3.reset(new PayloadBook<DummyLevel>(p3)));
        REQUIRE(ptr3->capacity == 1);
        REQUIRE(ptr3->capacity == p3.capacity);
        REQUIRE(ptr3->empty<side::bid>());
        REQUIRE(not ptr3->full<side::bid>());
        REQUIRE(ptr3->empty<side::ask>());
        REQUIRE(not ptr3->full<side::ask>());
    }

    SECTION("reuse single pointer for different data stores") {
        std::unique_ptr<PayloadBook<ConstLevel>> ptr = nullptr;
        PayloadBook<ConstLevel>::data<127> p2a = {};
        CHECK_NOTHROW(ptr.reset(new PayloadBook<ConstLevel>(p2a)));
        REQUIRE(ptr->capacity == 127);
        REQUIRE(ptr->capacity == p2a.capacity);
        REQUIRE(ptr->size<side::bid>() == 0);
        REQUIRE(ptr->size<side::ask>() == 0);
        CHECK(ptr->empty<side::bid>());
        CHECK(not ptr->full<side::bid>());
        for (int i = 0; i < 126; ++i) {
            REQUIRE(ptr->emplace_back<side::bid>(120'000 - i, 200) == i);
            REQUIRE(ptr->size<side::bid>() == i + 1);
            CHECK(not ptr->empty<side::bid>());
            CHECK(not ptr->full<side::bid>());
        }
        REQUIRE(ptr->emplace_back<side::bid>(120'000 - 127, 200) == 126);
        REQUIRE(ptr->size<side::bid>() == 127);
        CHECK(not ptr->empty<side::bid>());
        CHECK(ptr->full<side::bid>());
        CHECK(ptr->emplace_back<side::bid>() == PayloadBook<ConstLevel>::npos);
        for (int i = 0; i < 127; ++i) {
            REQUIRE(ptr->emplace_back<side::ask>(120'000 + i, 200) == i);
            REQUIRE(ptr->size<side::ask>() == i + 1);
            CHECK(not ptr->empty<side::ask>());
        }
        CHECK(ptr->full<side::ask>());
        CHECK(ptr->emplace_back<side::ask>() == ptr->npos);

        PayloadBook<ConstLevel>::data<5> p2b = {};
        CHECK_NOTHROW(ptr.reset(new PayloadBook<ConstLevel>(p2b)));
        REQUIRE(ptr->capacity == 5);
        REQUIRE(ptr->capacity == p2b.capacity);
        REQUIRE(ptr->size<side::bid>() == 0);
        REQUIRE(ptr->size<side::ask>() == 0);
        REQUIRE(ptr->emplace_back<side::bid>(88858, 400) == 0);
        REQUIRE(ptr->emplace_back<side::bid>(88857, 400) == 1);
        REQUIRE(ptr->emplace_back<side::bid>(88856, 400) == 2);
        REQUIRE(ptr->emplace_back<side::bid>(88855, 400) == 3);
        REQUIRE(ptr->emplace_back<side::bid>(88854, 400) == 4);
        REQUIRE(ptr->emplace_back<side::bid>(88853, 400) == ptr->npos);
        REQUIRE(ptr->emplace_back<side::ask>(88857, 400) == 0);
        REQUIRE(ptr->emplace_back<side::ask>(88858, 400) == 1);
        REQUIRE(ptr->emplace_back<side::ask>(88859, 400) == 2);
        REQUIRE(ptr->emplace_back<side::ask>(88859, 400) == 3);
        REQUIRE(ptr->emplace_back<side::ask>(88859, 400) == 4);
        REQUIRE(ptr->emplace_back<side::ask>(88860, 400) == ptr->npos);
        ptr.reset();

        // At this point we expect that both p2a and p2b kept their data
        for (auto &l : p2a.levels) {
            CHECK(l.ticks >= 120'000 - 127);
            CHECK(l.ticks <= 120'000 + 127);
            CHECK(l.size == 200);
        }

        for (auto &l : p2b.levels) {
            CHECK(l.ticks >= 88853);
            CHECK(l.ticks <= 88859);
            CHECK(l.size == 400);
        }
    }
}
