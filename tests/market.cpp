// Copyright (c) 2018 Bronislaw (Bronek) Kozicki
//
// Distributed under the MIT License. See accompanying file LICENSE
// or copy at https://opensource.org/licenses/MIT

#include <market/market.hpp>

#include <catch2/catch.hpp>

TEST_CASE("BidAsk", "[sides]") {
    static_assert((size_t)market::side::bid == 0);
    static_assert((size_t)market::side::ask == 1);
    SUCCEED();
}
