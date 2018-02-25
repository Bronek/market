#include <book.hpp>

#include <catch.hpp>

TEST_CASE("Dummy", "[dummy][placeholder]") {
    market::book b;
    const auto* const ptr = &b;
    REQUIRE(ptr != nullptr);
}
