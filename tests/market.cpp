#include <market/market.hpp>

#include <catch2/catch.hpp>

TEST_CASE("BidAsk", "[sides]") {
    static_assert((size_t)market::side::bid == 0);
    static_assert((size_t)market::side::ask == 1);
    SUCCEED();
}
