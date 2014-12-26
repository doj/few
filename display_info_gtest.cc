/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#include "gtest/gtest.h"
#include "display_info.h"

namespace {
    lineNum_vector_t s()
    {
	lineNum_vector_t a;
	for(unsigned i = 1; i <= 100; ++i) {
	    a.push_back(i);
	}
	return a;
    }
}

TEST(DisplayInfo, size)
{
    DisplayInfo i; i.assign(s());
    ASSERT_EQ(100u, i.size());
}

TEST(DisplayInfo, lastLineNum)
{
    DisplayInfo i; i.assign(s());
    ASSERT_EQ(100u, i.lastLineNum());
}

TEST(DisplayInfo, go_to)
{
    DisplayInfo i; i.assign(s());
    ASSERT_TRUE(i.go_to(1));
    ASSERT_TRUE(i.go_to(50));
    ASSERT_TRUE(i.go_to(100));
    ASSERT_FALSE(i.go_to(101));
    ASSERT_FALSE(i.go_to(0));
}

TEST(DisplayInfo, down)
{
    DisplayInfo i; i.assign(s());

    i.down();
    i.down();
    i.down();

    i.start();
    ASSERT_EQ(4u, i.current());
}

TEST(DisplayInfo, down_does_not_go_too_low)
{
    DisplayInfo i; i.assign(s());

    for(unsigned u = 0; u < 1000; ++u) {
	i.down();
    }

    i.start();
    ASSERT_EQ(100u, i.current());
}

TEST(DisplayInfo, down_does_not_go_too_low2)
{
    // set info object to 100 lines
    DisplayInfo i; i.assign(s());

    // now set it to zero lines
    lineNum_vector_t zero;
    i.assign(std::move(zero));;

    ASSERT_FALSE(i.start());
    ASSERT_EQ(0u, i.current());
    i.down();
    ASSERT_EQ(0u, i.current());
}

TEST(DisplayInfo, up_does_not_go_through_the_ceiling)
{
    DisplayInfo i; i.assign(s());

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
    DisplayInfo i; i.assign(s());

    i.start();
    i.down();
    i.down();
    i.start();
    ASSERT_EQ(3u, i.current());

    i.assign(s());
    i.start();
    ASSERT_EQ(3u, i.current());
    i.down();
    i.down();
    i.down();
    i.down();
    i.start();
    ASSERT_EQ(7u, i.current());

    // a set with only every 5th line
    lineNum_vector_t a;
    for(unsigned i = 1; i <= 100; i += 5) {
	a.push_back(i);
    }
    i.assign(std::move(a));
    i.start();
    ASSERT_EQ(1u + 5u, i.current());

    // a small set, move upward
    a.clear();
    a.push_back(2);
    a.push_back(4);
    i.assign(std::move(a));
    i.start();
    ASSERT_EQ(4u, i.current());
}

TEST(DisplayInfo, topLineNum)
{
    DisplayInfo i;
    ASSERT_EQ(0u, i.topLineNum());
    i.assign(s());
    ASSERT_EQ(1u, i.topLineNum());
}

TEST(DisplayInfo, prev)
{
    DisplayInfo i;
    i.assign(s());
    i.start();
    ASSERT_FALSE(i.prev());

    ASSERT_TRUE(i.next());
    ASSERT_EQ(2u, i.current());
    ASSERT_TRUE(i.prev());
    ASSERT_EQ(1u, i.current());
    ASSERT_FALSE(i.prev());
}
