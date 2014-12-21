/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#include "gtest/gtest.h"
#include "regex_index.h"

TEST(regex_index, does_filter_non_empty_lines)
{
    auto f_idx = std::make_shared<file_index>("test.txt");
    regex_index r_idx(f_idx, "\\w", "");
    ASSERT_GE(r_idx.size(), 3u);
}

TEST(regex_index, i_flag)
{
    auto f_idx = std::make_shared<file_index>("test.txt");
    regex_index r_idx(f_idx, "this is line #10.", "i");
    ASSERT_EQ(1u, r_idx.size());
}

TEST(regex_index, not_flag)
{
    auto f_idx = std::make_shared<file_index>("test.txt");
    regex_index r_idx(f_idx, "#", "!");
    ASSERT_GE(r_idx.size(), 3u);
}

TEST(regex_index, intersect_works)
{
    auto f_idx = std::make_shared<file_index>("test.txt");

    regex_index a(f_idx, "#", "!");

    regex_index b(f_idx, "contains", "");
    auto s = b.intersect(a);

    ASSERT_EQ(1u, s.size());
}
