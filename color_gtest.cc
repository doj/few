/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#include "gtest/gtest.h"
#include "color.h"

TEST(Color, use_color_works)
{
    use_color(true);
    ASSERT_TRUE(use_color());
    use_color(false);
    ASSERT_FALSE(use_color());
    use_color(true);
    ASSERT_TRUE(use_color());
}

TEST(Color, color_returns_0_if_color_is_disabled)
{
    use_color(false);
    ASSERT_EQ(0u, color(2,2));
    ASSERT_EQ(0u, color(1,0));
}

TEST(Color, color_returns_useful_color_pairs)
{
    use_color(true);
    const uint64_t cp1 = color(2,2);
    ASSERT_GT(cp1, 0u);

    const uint64_t cp2 = color(1,0);
    ASSERT_GT(cp2, 0u);
    ASSERT_NE(cp1, cp2);

    const uint64_t cp3 = color(2,2);
    ASSERT_GT(cp1, 0u);
    ASSERT_EQ(cp1, cp3);
}
