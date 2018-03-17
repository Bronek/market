// Copyright (c) 2018 Bronislaw (Bronek) Kozicki
//
// Distributed under the MIT License. See accompanying file LICENSE
// or copy at https://opensource.org/licenses/MIT

#pragma once

#include <cstddef>

namespace market {
    // Used for indexing, so give it appropriate underlying type
    enum class side : size_t { bid = 0, ask = 1 };

    template <typename Level, typename Policy> struct book;
} // namespace market
