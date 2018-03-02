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
        SmallBook() : book<Level>(data)
        { }

        book::data<3> data;
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

namespace {
    template <typename Level>
    struct PayloadBook : market::book<Level> {
        template <int Size>
        using data = typename market::book<Level>::template data<Size>;

        template <int Size>
        explicit constexpr PayloadBook(data<Size>& p) : market::book<Level>(p)
        { }
    };
}

TEST_CASE("PayloadBook_stack", "[book][construction][book_data][stack][Validation]") {
    using namespace market;

    PayloadBook<ConstLevel>::data<4> p4 = {};
    // Calculate index of last element in p4.levels from capacity
    REQUIRE(p4.capacity == 4);
    const int back = p4.capacity * 2 - 1;
    REQUIRE(back == 7);
    REQUIRE(sizeof(p4.levels) / sizeof(p4.levels[0]) == back + 1);
    SECTION("regular constructor") {
        PayloadBook<ConstLevel> b4{p4};
        REQUIRE(b4.capacity == 4);
        CHECK(b4.empty<side::bid>());
        REQUIRE(b4.push_back<side::bid>(120120, 200) == 0);
        REQUIRE(b4.size<side::bid>() == 1);
        // Verify the newly inserted level is stored (somewhere) inside p4.levels
        const auto* ptr = &b4.at<side::bid>(0);
        REQUIRE(ptr >= &p4.levels[0]);
        REQUIRE(ptr <= &p4.levels[back]);
        // More extensive check below
    }

    bool populated = false;
    // Use SECTION scope to destroy b4 and CHECK that p4.levels remain in place, as populated by b4
    SECTION("reference lifetime extension") {
        auto &&b4 = PayloadBook<ConstLevel>(p4);
        REQUIRE(b4.capacity == 4);
        CHECK(b4.empty<side::ask>());
        CHECK(not b4.full<side::ask>());

        // Add elements on ask side
        REQUIRE(b4.push_back<side::ask>(120121, 300) == 0);
        REQUIRE(b4.size<side::ask>() == 1);
        CHECK(not b4.empty<side::ask>());
        CHECK(not b4.full<side::ask>());
        // Verify the newly inserted level is stored inside p4.levels
        const auto *ptr0a = &b4.at<side::ask>(0);
        REQUIRE(ptr0a >= &p4.levels[0]);
        REQUIRE(ptr0a <= &p4.levels[back]);
        CHECK(*ptr0a == ConstLevel{120121, 300});

        REQUIRE(b4.push_back<side::ask>(120122, 300) == 1);
        REQUIRE(b4.size<side::ask>() == 2);
        CHECK(not b4.empty<side::ask>());
        CHECK(not b4.full<side::ask>());
        const auto *ptr1a = &b4.at<side::ask>(1);
        REQUIRE(ptr1a >= &p4.levels[0]);
        REQUIRE(ptr1a <= &p4.levels[back]);
        CHECK(*ptr1a == ConstLevel{120122, 300});
        REQUIRE(ptr0a != ptr1a);

        REQUIRE(b4.push_back<side::ask>(120123, 300) == 2);
        REQUIRE(b4.size<side::ask>() == 3);
        CHECK(not b4.empty<side::ask>());
        CHECK(not b4.full<side::ask>());
        const auto *ptr2a = &b4.at<side::ask>(2);
        REQUIRE(ptr2a >= &p4.levels[0]);
        REQUIRE(ptr2a <= &p4.levels[back]);
        CHECK(*ptr2a == ConstLevel{120123, 300});
        REQUIRE(ptr0a != ptr2a);
        REQUIRE(ptr1a != ptr2a);

        REQUIRE(b4.push_back<side::ask>(120124, 300) == 3);
        REQUIRE(b4.size<side::ask>() == 4);
        CHECK(not b4.empty<side::ask>());
        CHECK(b4.full<side::ask>());
        const auto *ptr3a = &b4.at<side::ask>(3);
        REQUIRE(ptr3a >= &p4.levels[0]);
        REQUIRE(ptr3a <= &p4.levels[back]);
        CHECK(*ptr3a == ConstLevel{120124, 300});
        REQUIRE(ptr0a != ptr3a);
        REQUIRE(ptr1a != ptr3a);
        REQUIRE(ptr2a != ptr3a);
        CHECK(b4.push_back<side::ask>(120125, 500) == b4.npos);

        // Add elements on bid side
        REQUIRE(b4.push_back<side::bid>(120120, 300) == 0);
        REQUIRE(b4.size<side::bid>() == 1);
        CHECK(not b4.empty<side::bid>());
        CHECK(not b4.full<side::bid>());
        const auto *ptr0b = &b4.at<side::bid>(0);
        REQUIRE(ptr0b >= &p4.levels[0]);
        REQUIRE(ptr0b <= &p4.levels[back]);
        CHECK(*ptr0b == ConstLevel{120120, 300});
        REQUIRE(ptr0a != ptr0b);
        REQUIRE(ptr1a != ptr0b);
        REQUIRE(ptr2a != ptr0b);
        REQUIRE(ptr3a != ptr0b);

        REQUIRE(b4.push_back<side::bid>(120119, 300) == 1);
        REQUIRE(b4.size<side::bid>() == 2);
        CHECK(not b4.empty<side::bid>());
        CHECK(not b4.full<side::bid>());
        const auto *ptr1b = &b4.at<side::bid>(1);
        REQUIRE(ptr1b >= &p4.levels[0]);
        REQUIRE(ptr1b <= &p4.levels[back]);
        CHECK(*ptr1b == ConstLevel{120119, 300});
        REQUIRE(ptr0a != ptr1b);
        REQUIRE(ptr1a != ptr1b);
        REQUIRE(ptr2a != ptr1b);
        REQUIRE(ptr3a != ptr1b);
        REQUIRE(ptr0b != ptr1b);

        REQUIRE(b4.push_back<side::bid>(120118, 300) == 2);
        REQUIRE(b4.size<side::bid>() == 3);
        CHECK(not b4.empty<side::bid>());
        CHECK(not b4.full<side::bid>());
        const auto *ptr2b = &b4.at<side::bid>(2);
        REQUIRE(ptr2b >= &p4.levels[0]);
        REQUIRE(ptr2b <= &p4.levels[back]);
        CHECK(*ptr2b == ConstLevel{120118, 300});
        REQUIRE(ptr0a != ptr2b);
        REQUIRE(ptr1a != ptr2b);
        REQUIRE(ptr2a != ptr2b);
        REQUIRE(ptr3a != ptr2b);
        REQUIRE(ptr0b != ptr2b);
        REQUIRE(ptr1b != ptr2b);

        REQUIRE(b4.push_back<side::bid>(120117, 300) == 3);
        REQUIRE(b4.size<side::bid>() == 4);
        CHECK(not b4.empty<side::bid>());
        CHECK(b4.full<side::bid>());
        const auto *ptr3b = &b4.at<side::bid>(3);
        REQUIRE(ptr3b >= &p4.levels[0]);
        REQUIRE(ptr3b <= &p4.levels[back]);
        CHECK(*ptr3b == ConstLevel{120117, 300});
        REQUIRE(ptr0a != ptr3b);
        REQUIRE(ptr1a != ptr3b);
        REQUIRE(ptr2a != ptr3b);
        REQUIRE(ptr3a != ptr3b);
        REQUIRE(ptr0b != ptr3b);
        REQUIRE(ptr1b != ptr3b);
        REQUIRE(ptr2b != ptr3b);
        CHECK(b4.push_back<side::bid>(120116, 500) == b4.npos);

        populated = true;
    }

    if (populated) {
        // At this point we expect that p4.levels array is fully populated
        for (auto &l : p4.levels) {
            CHECK(l.ticks >= 120117);
            CHECK(l.ticks <= 120124);
            CHECK(l.size == 300);
        }
    }
}

TEST_CASE("PayloadBook_heap", "[book][construction][book_data][heap][Validation]") {
    using namespace market;

    SECTION("regular capacity book, compile-time validation of capacity") {
        std::unique_ptr<PayloadBook<Level>> ptr1 = nullptr;
        PayloadBook<Level>::data<20> p1 = {};
        CHECK_NOTHROW(ptr1.reset(new PayloadBook<Level>(p1)));
        REQUIRE(ptr1->capacity == 20);
        REQUIRE(ptr1->capacity == p1.capacity);
        REQUIRE(ptr1->size<side::bid>() == 0);
        REQUIRE(ptr1->size<side::ask>() == 0);
        REQUIRE(ptr1->push_back<side::bid>() == 0);
        REQUIRE(ptr1->push_back<side::ask>() == 0);

        std::unique_ptr<PayloadBook<DummyLevel>> ptr3 = nullptr;
        PayloadBook<DummyLevel>::data<1> p3 = {};
        CHECK_NOTHROW(ptr3.reset(new PayloadBook<DummyLevel>(p3)));
        REQUIRE(ptr3->capacity == 1);
        REQUIRE(ptr3->capacity == p3.capacity);
        REQUIRE(ptr3->size<side::bid>() == 0);
        REQUIRE(ptr3->size<side::ask>() == 0);
        REQUIRE(ptr3->push_back<side::bid>() == 0);
        REQUIRE(ptr3->push_back<side::ask>() == 0);
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
            REQUIRE(ptr->push_back<side::bid>(120'000 - i, 200) == i);
            REQUIRE(ptr->size<side::bid>() == i + 1);
            CHECK(not ptr->empty<side::bid>());
            CHECK(not ptr->full<side::bid>());
        }
        REQUIRE(ptr->push_back<side::bid>(120'000 - 127, 200) == 126);
        REQUIRE(ptr->size<side::bid>() == 127);
        CHECK(not ptr->empty<side::bid>());
        CHECK(ptr->full<side::bid>());
        CHECK(ptr->push_back<side::bid>() == PayloadBook<ConstLevel>::npos);
        for (int i = 0; i < 127; ++i) {
            REQUIRE(ptr->push_back<side::ask>(120'000 + i, 200) == i);
            REQUIRE(ptr->size<side::ask>() == i + 1);
            CHECK(not ptr->empty<side::ask>());
        }
        CHECK(ptr->full<side::ask>());
        CHECK(ptr->push_back<side::ask>() == ptr->npos);

        PayloadBook<ConstLevel>::data<5> p2b = {};
        CHECK_NOTHROW(ptr.reset(new PayloadBook<ConstLevel>(p2b)));
        REQUIRE(ptr->capacity == 5);
        REQUIRE(ptr->capacity == p2b.capacity);
        REQUIRE(ptr->size<side::bid>() == 0);
        REQUIRE(ptr->size<side::ask>() == 0);
        CHECK(ptr->empty<side::bid>());
        REQUIRE(ptr->push_back<side::bid>(88858, 200) == 0);
        CHECK(not ptr->empty<side::bid>());
        REQUIRE(ptr->push_back<side::bid>(88857, 200) == 1);
        REQUIRE(ptr->push_back<side::bid>(88856, 200) == 2);
        REQUIRE(ptr->push_back<side::bid>(88855, 200) == 3);
        CHECK(not ptr->full<side::bid>());
        REQUIRE(ptr->push_back<side::bid>(88854, 200) == 4);
        CHECK(ptr->full<side::bid>());
        REQUIRE(ptr->push_back<side::bid>(88853, 200) == ptr->npos);

        // At this point we expect that p2a.levels kept its data
        for (auto &l : p2a.levels) {
            CHECK(l.ticks >= 120'000 - 127);
            CHECK(l.ticks <= 120'000 + 127);
            CHECK(l.size == 200);
        }
    }
}
