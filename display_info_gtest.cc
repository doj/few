/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#include "gtest/gtest.h"
#include "display_info.h"

namespace {
    lineNum_set_t s()
    {
	lineNum_set_t a;
	for(unsigned i = 1; i <= 100; ++i) {
	    a.insert(i);
	}
	return a;
    }
}

TEST(DisplayInfo, size)
{
    DisplayInfo i; i = s();
    ASSERT_EQ(100u, i.size());
}


TEST(DisplayInfo, down)
{
    DisplayInfo i; i = s();

    i.down();
    i.down();
    i.down();

    i.start();
    ASSERT_EQ(4u, i.current());
}

TEST(DisplayInfo, down_does_not_go_too_low)
{
    DisplayInfo i; i = s();

    for(unsigned u = 0; u < 1000; ++u) {
	i.down();
    }

    i.start();
    ASSERT_EQ(100u, i.current());
}

TEST(DisplayInfo, up_does_not_go_through_the_ceiling)
{
    DisplayInfo i; i = s();

    i.start();
    ASSERT_EQ(1u, i.current());

    i.up();
    i.start();
    ASSERT_EQ(1u, i.current());

    i.down();
    i.down();
    i.start();
    ASSERT_EQ(3u, i.current());

    i.up();
    i.up();
    i.up();
    i.up();
    i.up();
    i.up();
    i.up();
    i.start();
    ASSERT_EQ(1u, i.current());
}
