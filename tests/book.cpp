#include <book.hpp>

#include <catch.hpp>

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
        SmallBook() : book<Level>(levels, sides, 3), levels{}, sides{}
        { }

        Level levels[6];
        size_type sides[6];
    };
}

TEST_CASE("SmallBook", "[book][push_back][size][full][empty][at][DuplicateData]") {
    using namespace market;
    SmallBook book;
    CHECK(book.capacity == 3);
    CHECK(book.size<side::bid>() == 0);
    CHECK(book.empty<side::bid>());
    CHECK(not book.full<side::bid>());

    CHECK(book.size<side::ask>() == 0);
    CHECK(book.empty<side::ask>());
    CHECK(not book.full<side::ask>());

    REQUIRE(book.push_back<side::bid>(Level{120120, 100}) == 0);
    REQUIRE(book.size<side::bid>() == 1);
    CHECK(book.at<side::bid>(0) == Level{120120, 100});
    CHECK(not book.empty<side::bid>());
    CHECK(not book.full<side::bid>());

    CHECK(book.size<side::ask>() == 0);
    CHECK(book.empty<side::ask>());
    CHECK(not book.full<side::ask>());

    REQUIRE(book.push_back<side::bid>(Level{120121, 100}) == 1);
    REQUIRE(book.size<side::bid>() == 2);
    CHECK(book.at<side::bid>(0) == Level{120120, 100});
    CHECK(book.at<side::bid>(1) == Level{120121, 100});
    CHECK(not book.empty<side::bid>());
    CHECK(not book.full<side::bid>());

    CHECK(book.size<side::ask>() == 0);
    CHECK(book.empty<side::ask>());
    CHECK(not book.full<side::ask>());

    SECTION("repeated level, full bid side") {
        REQUIRE(book.push_back<side::bid>(Level{120121, 100}) == 2); // Same level ticks as before
        REQUIRE(book.size<side::bid>() == 3);
        CHECK(book.at<side::bid>(0) == Level{120120, 100});
        CHECK(book.at<side::bid>(1) == Level{120121, 100});
        CHECK(book.at<side::bid>(2) == Level{120121, 100});
        CHECK(not book.empty<side::bid>());
        CHECK(book.full<side::bid>());

        CHECK(book.size<side::ask>() == 0);
        CHECK(book.empty<side::ask>());
        CHECK(not book.full<side::ask>());

        CHECK(book.push_back<side::bid>(Level{120123, 100}) == SmallBook::npos);
        REQUIRE(book.size<side::bid>() == 3);
        CHECK(book.at<side::bid>(0) == Level{120120, 100});
        CHECK(book.at<side::bid>(1) == Level{120121, 100});
        CHECK(book.at<side::bid>(2) == Level{120121, 100});
        CHECK(not book.empty<side::bid>());
        CHECK(book.full<side::bid>());

        CHECK(book.size<side::ask>() == 0);
        CHECK(book.empty<side::ask>());
        CHECK(not book.full<side::ask>());
    }

    SECTION("another repeated level, full bid side") {
        REQUIRE(book.push_back<side::bid>(Level{120120, 100}) == 2); // Same level ticks as earlier
        REQUIRE(book.size<side::bid>() == 3);
        CHECK(const_cast<const SmallBook&>(book).at<side::bid>(0) == Level{120120, 100});
        CHECK(const_cast<const SmallBook&>(book).at<side::bid>(1) == Level{120121, 100});
        CHECK(const_cast<const SmallBook&>(book).at<side::bid>(2) == Level{120120, 100});
        CHECK(not book.empty<side::bid>());
        CHECK(book.full<side::bid>());

        CHECK(book.size<side::ask>() == 0);
        CHECK(book.empty<side::ask>());
        CHECK(not book.full<side::ask>());

        CHECK(book.push_back<side::bid>(Level{120110, 100}) == SmallBook::npos);
        REQUIRE(book.size<side::bid>() == 3);
        CHECK(book.at<side::bid>(0) == Level{120120, 100});
        CHECK(book.at<side::bid>(1) == Level{120121, 100});
        CHECK(book.at<side::bid>(2) == Level{120120, 100});
        CHECK(not book.empty<side::bid>());
        CHECK(book.full<side::bid>());

        CHECK(book.size<side::ask>() == 0);
        CHECK(book.empty<side::ask>());
        CHECK(not book.full<side::ask>());
    }

    SECTION("different level, full bid side") {
        REQUIRE(book.push_back<side::bid>(Level{120122, 100}) == 2);
        REQUIRE(book.size<side::bid>() == 3);
        CHECK(book.at<side::bid>(0) == Level{120120, 100});
        CHECK(book.at<side::bid>(1) == Level{120121, 100});
        CHECK(book.at<side::bid>(2) == Level{120122, 100});
        CHECK(not book.empty<side::bid>());
        CHECK(book.full<side::bid>());

        CHECK(book.size<side::ask>() == 0);
        CHECK(book.empty<side::ask>());
        CHECK(not book.full<side::ask>());

        CHECK(book.push_back<side::bid>(Level{120122, 100}) == SmallBook::npos);
        REQUIRE(book.size<side::bid>() == 3);
        CHECK(const_cast<const SmallBook&>(book).at<side::bid>(0) == Level{120120, 100});
        CHECK(const_cast<const SmallBook&>(book).at<side::bid>(1) == Level{120121, 100});
        CHECK(const_cast<const SmallBook&>(book).at<side::bid>(2) == Level{120122, 100});
        CHECK(not book.empty<side::bid>());
        CHECK(book.full<side::bid>());

        CHECK(book.size<side::ask>() == 0);
        CHECK(book.empty<side::ask>());
        CHECK(not book.full<side::ask>());
    }

    SECTION("fill both sides") {
        REQUIRE(book.size<side::bid>() == 2);
        CHECK(book.at<side::bid>(0) == Level{120120, 100});
        CHECK(book.at<side::bid>(1) == Level{120121, 100});
        CHECK(not book.empty<side::bid>());
        CHECK(not book.full<side::bid>());

        REQUIRE(book.push_back<side::ask>(Level{120124, 200}) == 0);
        REQUIRE(book.size<side::ask>() == 1);
        CHECK(book.at<side::ask>(0) == Level{120124, 200});
        CHECK(not book.empty<side::ask>());
        CHECK(not book.full<side::ask>());

        REQUIRE(book.push_back<side::ask>(Level{120125, 300}) == 1);
        REQUIRE(book.size<side::ask>() == 2);
        CHECK(book.at<side::ask>(1) == Level{120125, 300});
        CHECK(not book.empty<side::ask>());
        CHECK(not book.full<side::ask>());

        REQUIRE(book.push_back<side::ask>(Level{120126, 400}) == 2);
        REQUIRE(book.size<side::ask>() == 3);
        CHECK(book.at<side::ask>(2) == Level{120126, 400});
        CHECK(not book.empty<side::ask>());
        CHECK(book.full<side::ask>());
        // Ask side is now full

        REQUIRE(book.size<side::bid>() == 2);
        CHECK(book.at<side::bid>(0) == Level{120120, 100});
        CHECK(book.at<side::bid>(1) == Level{120121, 100});
        CHECK(not book.empty<side::bid>());
        CHECK(not book.full<side::bid>());

        CHECK(book.push_back<side::ask>(Level{120121, 100}) == SmallBook::npos);
        REQUIRE(book.size<side::ask>() == 3);
        CHECK(book.at<side::ask>(0) == Level{120124, 200});
        CHECK(book.at<side::ask>(1) == Level{120125, 300});
        CHECK(book.at<side::ask>(2) == Level{120126, 400});
        CHECK(not book.empty<side::ask>());
        CHECK(book.full<side::ask>());

        CHECK(book.size<side::bid>() == 2);
        CHECK(not book.empty<side::bid>());
        CHECK(not book.full<side::bid>());

        REQUIRE(book.push_back<side::bid>(Level{120125, 200}) == 2);
        REQUIRE(book.size<side::bid>() == 3);
        CHECK(book.at<side::bid>(0) == Level{120120, 100});
        CHECK(book.at<side::bid>(1) == Level{120121, 100});
        CHECK(book.at<side::bid>(2) == Level{120125, 200});
        CHECK(not book.empty<side::bid>());
        CHECK(book.full<side::bid>());
        // Bid side is now full

        REQUIRE(book.size<side::ask>() == 3);
        CHECK(book.at<side::ask>(0) == Level{120124, 200});
        CHECK(book.at<side::ask>(1) == Level{120125, 300});
        CHECK(book.at<side::ask>(2) == Level{120126, 400});
        CHECK(not book.empty<side::ask>());
        CHECK(book.full<side::ask>());

        CHECK(book.push_back<side::bid>(Level{120121, 100}) == SmallBook::npos);
        REQUIRE(book.size<side::bid>() == 3);
        CHECK(book.at<side::bid>(0) == Level{120120, 100});
        CHECK(book.at<side::bid>(1) == Level{120121, 100});
        CHECK(book.at<side::bid>(2) == Level{120125, 200});
        CHECK(not book.empty<side::bid>());
        CHECK(book.full<side::bid>());

        CHECK(book.size<side::ask>() == 3);
        CHECK(not book.empty<side::ask>());
        CHECK(book.full<side::ask>());
    }

    SECTION("emplace-like initialisation of level") {
        REQUIRE(book.size<side::ask>() == 0);
        CHECK(book.empty<side::ask>());
        CHECK(not book.full<side::ask>());

        REQUIRE(book.push_back<side::ask>(120122, 500) == 0);
        REQUIRE(book.size<side::ask>() == 1);
        CHECK(book.at<side::ask>(0) == Level{120122, 500});
        CHECK(not book.empty<side::ask>());
        CHECK(not book.full<side::ask>());

        REQUIRE(book.push_back<side::ask>(120123, 150) == 1);
        REQUIRE(book.size<side::ask>() == 2);
        CHECK(book.at<side::ask>(1) == Level{120123, 150});
        CHECK(not book.empty<side::ask>());
        CHECK(not book.full<side::ask>());

        REQUIRE(book.push_back<side::ask>(120124, 200) == 2);
        REQUIRE(book.size<side::ask>() == 3);
        CHECK(book.at<side::ask>(2) == Level{120124, 200});
        CHECK(not book.empty<side::ask>());
        CHECK(book.full<side::ask>());

        CHECK(book.push_back<side::ask>(120125, 200) == SmallBook::npos);
        REQUIRE(book.size<side::ask>() == 3);
    }
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
        SmallConstBook() : book<ConstLevel>(levels, sides, 2, nothrow), levels{}, sides{}
        { }

        ConstLevel levels[4];
        size_type sides[4];
    };
}

TEST_CASE("SmallConstBook", "[book][push_back][size][full][empty][at][Emplace][Immutable][DuplicateData]") {
    using namespace market;
    SmallConstBook book;
    CHECK(book.capacity == 2);
    CHECK(book.empty<side::bid>());
    CHECK(not book.full<side::bid>());

    CHECK(book.empty<side::ask>());
    CHECK(not book.full<side::ask>());
    // Test that emplace-like push_back works with immutable levels
    REQUIRE(book.push_back<side::bid>(120120, 200) == 0);
    REQUIRE(book.size<side::bid>() == 1);
    CHECK(book.at<side::bid>(0) == ConstLevel{120120, 200});
    CHECK(not book.empty<side::bid>());
    CHECK(not book.full<side::bid>());

    CHECK(book.empty<side::ask>());
    CHECK(not book.full<side::ask>());

    REQUIRE(book.push_back<side::bid>(120118, 500) == 1);
    REQUIRE(book.size<side::bid>() == 2);
    CHECK(book.at<side::bid>(0) == ConstLevel{120120, 200});
    CHECK(book.at<side::bid>(1) == ConstLevel{120118, 500});
    CHECK(not book.empty<side::bid>());
    CHECK(book.full<side::bid>());

    CHECK(book.empty<side::ask>());
    CHECK(not book.full<side::ask>());

    CHECK(book.push_back<side::bid>(120118, 500) == SmallConstBook::npos);
    REQUIRE(book.size<side::bid>() == 2);
    CHECK(book.at<side::bid>(0) == ConstLevel{120120, 200});
    CHECK(book.at<side::bid>(1) == ConstLevel{120118, 500});
    CHECK(not book.empty<side::bid>());
    CHECK(book.full<side::bid>());

    auto& level = book.at<side::bid>(0);
    static_assert(std::is_same_v<decltype(level.ticks), const int>);
    static_assert(std::is_same_v<decltype(level.size), const int>);
}

namespace {
    struct DummyLevel {};

    struct PretendSizeBook : market::book<DummyLevel> {
        DummyLevel levels[254] = {};
        uint8_t sides[254] = {};

        explicit PretendSizeBook(int i) : book(levels, sides, i)
        { }

        template <typename Tag>
        PretendSizeBook(int i, Tag&& t) : book(levels, sides, i, std::forward<Tag>(t))
        { }
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
        REQUIRE(ptr->push_back<side::bid>() == PretendSizeBook::npos);
        REQUIRE(ptr->push_back<side::ask>() == PretendSizeBook::npos);

        CHECK_NOTHROW(ptr.reset(new PretendSizeBook(0, PretendSizeBook::nothrow)));
        CHECK(ptr->capacity == 0);
        REQUIRE(ptr->size<side::bid>() == 0);
        REQUIRE(ptr->full<side::bid>());
        REQUIRE(ptr->empty<side::bid>());
        REQUIRE(ptr->size<side::ask>() == 0);
        REQUIRE(ptr->full<side::ask>());
        REQUIRE(ptr->empty<side::ask>());
        REQUIRE(ptr->push_back<side::bid>() == PretendSizeBook::npos);
        REQUIRE(ptr->push_back<side::ask>() == PretendSizeBook::npos);
    }

    SECTION("regular capacity book, no exceptions") {
        CHECK_NOTHROW(ptr.reset(new PretendSizeBook(40)));
        REQUIRE(ptr->capacity == 40);
        REQUIRE(ptr->size<side::bid>() == 0);
        REQUIRE(ptr->size<side::ask>() == 0);
        REQUIRE(ptr->push_back<side::bid>() == 0);
        REQUIRE(ptr->push_back<side::ask>() == 0);

        CHECK_NOTHROW(ptr.reset(new PretendSizeBook(126)));
        REQUIRE(ptr->capacity == 126);
        REQUIRE(ptr->size<side::bid>() == 0);
        REQUIRE(ptr->size<side::ask>() == 0);
        REQUIRE(ptr->push_back<side::bid>() == 0);
        REQUIRE(ptr->push_back<side::ask>() == 0);

        CHECK_NOTHROW(ptr.reset(new PretendSizeBook(127)));
        REQUIRE(ptr->capacity == 127);
        REQUIRE(ptr->size<side::bid>() == 0);
        REQUIRE(ptr->size<side::ask>() == 0);
        REQUIRE(ptr->push_back<side::bid>() == 0);
        REQUIRE(ptr->push_back<side::ask>() == 0);
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
        REQUIRE(ptr->push_back<side::bid>() == 0);
        REQUIRE(ptr->push_back<side::ask>() == 0);

        CHECK_NOTHROW(ptr.reset(new PretendSizeBook(126, PretendSizeBook::nothrow_t{})));
        REQUIRE(ptr->capacity == 126);
        REQUIRE(ptr->push_back<side::bid>() == 0);
        REQUIRE(ptr->push_back<side::ask>() == 0);

        CHECK_NOTHROW(ptr.reset(new PretendSizeBook(127, PretendSizeBook::nothrow)));
        REQUIRE(ptr->capacity == 127);
        REQUIRE(ptr->push_back<side::bid>() == 0);
        REQUIRE(ptr->push_back<side::ask>() == 0);
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
        REQUIRE(ptr->push_back<side::bid>() == PretendSizeBook::npos);
        REQUIRE(ptr->push_back<side::ask>() == PretendSizeBook::npos);

        CHECK_NOTHROW(ptr.reset(new PretendSizeBook(128, PretendSizeBook::nothrow)));
        CHECK(ptr->capacity == 0);
        REQUIRE(ptr->size<side::bid>() == 0);
        REQUIRE(ptr->full<side::bid>());
        REQUIRE(ptr->empty<side::bid>());
        REQUIRE(ptr->size<side::ask>() == 0);
        REQUIRE(ptr->full<side::ask>());
        REQUIRE(ptr->empty<side::ask>());
        REQUIRE(ptr->push_back<side::bid>() == PretendSizeBook::npos);
        REQUIRE(ptr->push_back<side::ask>() == PretendSizeBook::npos);

        CHECK_NOTHROW(ptr.reset(new PretendSizeBook(-127, PretendSizeBook::nothrow)));
        CHECK(ptr->capacity == 0);
        REQUIRE(ptr->size<side::bid>() == 0);
        REQUIRE(ptr->full<side::bid>());
        REQUIRE(ptr->empty<side::bid>());
        REQUIRE(ptr->size<side::ask>() == 0);
        REQUIRE(ptr->full<side::ask>());
        REQUIRE(ptr->empty<side::ask>());
        REQUIRE(ptr->push_back<side::bid>() == PretendSizeBook::npos);
        REQUIRE(ptr->push_back<side::ask>() == PretendSizeBook::npos);

        CHECK_NOTHROW(ptr.reset(new PretendSizeBook((int) 1e9, PretendSizeBook::nothrow)));
        CHECK(ptr->capacity == 0);
        REQUIRE(ptr->size<side::bid>() == 0);
        REQUIRE(ptr->full<side::bid>());
        REQUIRE(ptr->empty<side::bid>());
        REQUIRE(ptr->size<side::ask>() == 0);
        REQUIRE(ptr->full<side::ask>());
        REQUIRE(ptr->empty<side::ask>());
        REQUIRE(ptr->push_back<side::bid>() == PretendSizeBook::npos);
        REQUIRE(ptr->push_back<side::ask>() == PretendSizeBook::npos);

        CHECK_NOTHROW(ptr.reset(new PretendSizeBook(std::numeric_limits<int>::min(), PretendSizeBook::nothrow)));
        CHECK(ptr->capacity == 0);
        REQUIRE(ptr->size<side::bid>() == 0);
        REQUIRE(ptr->full<side::bid>());
        REQUIRE(ptr->empty<side::bid>());
        REQUIRE(ptr->size<side::ask>() == 0);
        REQUIRE(ptr->full<side::ask>());
        REQUIRE(ptr->empty<side::ask>());
        REQUIRE(ptr->push_back<side::bid>() == PretendSizeBook::npos);
        REQUIRE(ptr->push_back<side::ask>() == PretendSizeBook::npos);

        CHECK_NOTHROW(ptr.reset(new PretendSizeBook(std::numeric_limits<int>::max(), PretendSizeBook::nothrow)));
        CHECK(ptr->capacity == 0);
        REQUIRE(ptr->size<side::bid>() == 0);
        REQUIRE(ptr->full<side::bid>());
        REQUIRE(ptr->empty<side::bid>());
        REQUIRE(ptr->size<side::ask>() == 0);
        REQUIRE(ptr->full<side::ask>());
        REQUIRE(ptr->empty<side::ask>());
        REQUIRE(ptr->push_back<side::bid>() == PretendSizeBook::npos);
        REQUIRE(ptr->push_back<side::ask>() == PretendSizeBook::npos);
    }
}
