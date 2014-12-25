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

TEST(DisplayInfo, lastLineNum)
{
    DisplayInfo i; i = s();
    ASSERT_EQ(100u, i.lastLineNum());
}

TEST(DisplayInfo, go_to)
{
    DisplayInfo i; i = s();
    ASSERT_TRUE(i.go_to(1));
    ASSERT_TRUE(i.go_to(50));
    ASSERT_TRUE(i.go_to(100));
    ASSERT_FALSE(i.go_to(101));
    ASSERT_FALSE(i.go_to(0));
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

TEST(DisplayInfo, down_does_not_go_too_low2)
{
    // set info object to 100 lines
    DisplayInfo i; i = s();

    // now set it to zero lines
    lineNum_set_t zero;
    i = zero;

    ASSERT_FALSE(i.start());
    ASSERT_EQ(0u, i.current());
    i.down();
    ASSERT_EQ(0u, i.current());
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

TEST(DisplayInfo, stays_near_current_line_if_new_set_is_assigned)
{
    DisplayInfo i; i = s();

    i.start();
    i.down();
    i.down();
    i.start();
    ASSERT_EQ(3u, i.current());

    i = s();
    i.start();
    ASSERT_EQ(3u, i.current());
    i.down();
    i.down();
    i.down();
    i.down();
    i.start();
    ASSERT_EQ(7u, i.current());

    // a set with only every 5th line
    lineNum_set_t a;
    for(unsigned i = 1; i <= 100; i += 5) {
	a.insert(i);
    }
    i = a;
    i.start();
    ASSERT_EQ(1u + 5u, i.current());

    // a small set, move upward
    a.clear();
    a.insert(2);
    a.insert(4);
    i = a;
    i.start();
    ASSERT_EQ(4u, i.current());
}
