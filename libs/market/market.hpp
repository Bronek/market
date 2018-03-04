#pragma once

#include <cstddef>

namespace market {
    // Used for indexing, so give it appropriate underlying type
    enum class side : size_t { bid = 0, ask = 1 };

    template <typename Level> struct book;
} // namespace market
