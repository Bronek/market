// Copyright (c) 2018 Bronislaw (Bronek) Kozicki
//
// Distributed under the MIT License. See accompanying file LICENSE
// or copy at https://opensource.org/licenses/MIT

#include "common/utils.hpp"

#include <catch2/catch.hpp>

namespace {
    struct Dummy {
        const int i;

        Dummy() : i(0) { ctor_def++; }
        Dummy(const Dummy& s) : i(s.i) { ctor_cpy++; }
        Dummy(Dummy&& s) : i(s.i) { ctor_mov++; }
        Dummy(int i) : i(i) { ctor_int++; }
        ~Dummy() { dtor++; }

        static int ctor_def;
        static int ctor_cpy;
        static int ctor_mov;
        static int ctor_int;
        static int dtor;
    };

    int Dummy::ctor_def = 0;
    int Dummy::ctor_cpy = 0;
    int Dummy::ctor_mov = 0;
    int Dummy::ctor_int = 0;
    int Dummy::dtor = 0;
}

TEST_CASE("Emplace", "[emplace]") {
    using namespace common;

    // We are doing something naughty, but perfectly legal: replacing object d1 living
    // on stack with other objects of the same type and at the same memory location
    Dummy d1;

    CHECK(Dummy::ctor_def == 1);
    CHECK(Dummy::ctor_cpy == 0);
    CHECK(Dummy::ctor_mov == 0);
    CHECK(Dummy::ctor_int == 0);
    CHECK(Dummy::dtor == 0);
    CHECK(d1.i == 0);

    CHECK(common::emplace(&d1, 2)->i == 2);
    CHECK(Dummy::ctor_def == 1);
    CHECK(Dummy::ctor_cpy == 0);
    CHECK(Dummy::ctor_mov == 0);
    CHECK(Dummy::ctor_int == 1);
    CHECK(Dummy::dtor == 1);
    CHECK(d1.i == 2);

    CHECK(common::emplace(&d1, Dummy(3))->i == 3);
    CHECK(Dummy::ctor_def == 1);
    CHECK(Dummy::ctor_cpy == 0);
    CHECK(Dummy::ctor_mov == 1);
    CHECK(Dummy::ctor_int == 2);
    CHECK(Dummy::dtor == 3);
    CHECK(d1.i == 3);

    {
        Dummy tmp1;
        CHECK(common::emplace(&d1, std::move(tmp1))->i == 0);
        CHECK(Dummy::ctor_def == 2);
        CHECK(Dummy::ctor_cpy == 0);
        CHECK(Dummy::ctor_mov == 2);
        CHECK(Dummy::ctor_int == 2);
        CHECK(Dummy::dtor == 4);
        CHECK(d1.i == 0);
    }
    CHECK(Dummy::dtor == 5);

    const Dummy tmp2{5};
    CHECK(common::emplace(&d1, tmp2)->i == 5);
    CHECK(Dummy::ctor_def == 2);
    CHECK(Dummy::ctor_cpy == 1);
    CHECK(Dummy::ctor_mov == 2);
    CHECK(Dummy::ctor_int == 3);
    CHECK(Dummy::dtor == 6);
    CHECK(d1.i == 5);

    // We will emplace a Dummy object at the memory location of tmp3 array
    int tmp3[4] = {};
    auto* ptr3 = common::emplace<Dummy>((void*)tmp3, 8);
    CHECK((void*)ptr3 == (void*)&tmp3[0]);
    CHECK(ptr3->i == 8);
    CHECK(Dummy::ctor_def == 2);
    CHECK(Dummy::ctor_cpy == 1);
    CHECK(Dummy::ctor_mov == 2);
    CHECK(Dummy::ctor_int == 4);
    CHECK(Dummy::dtor == 6);
    CHECK(d1.i == 5);

    // This one line is (would be, if uncommented) very naughty, because we are referring to
    // tmp3[0] after it has been replaced by Dummy, living at the very same memory location
    // CHECK(tmp3[0] == 8); LEAVE IT OUT EVEN THOUGH IT "WORKS" !
}
