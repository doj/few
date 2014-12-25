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
    auto s = b.intersect(a.lineNum_set());
    ASSERT_EQ(1u, s.size());

    regex_index c(f_idx, "this does not match anything", "");
    ASSERT_EQ(0u, c.size());
    s = c.intersect(a.lineNum_set());
    ASSERT_EQ(0u, s.size());
}

TEST(regex_index, intersect_works_on_test2_txt)
{
    auto file = std::make_shared<file_index>("test2.txt");
    file->parse_all();
    ASSERT_EQ(6u, file->size());

    regex_index word_characters(file, "\\w", "");
    ASSERT_EQ(6u, word_characters.size());

    regex_index b(file, "b+", "");
    ASSERT_EQ(1u, b.size());

    auto s = file->lineNum_set();
    ASSERT_EQ(6u, s.size());
    s = word_characters.intersect(s);
    ASSERT_EQ(6u, s.size());
    s = b.intersect(s);
    ASSERT_EQ(1u, s.size());

    s = file->lineNum_set();
    ASSERT_EQ(6u, s.size());
    s = b.intersect(s);
    ASSERT_EQ(1u, s.size());
    s = word_characters.intersect(s);
    ASSERT_EQ(1u, s.size());

    regex_index z(file, "z", "");
    ASSERT_EQ(0u, z.size());
    s = file->lineNum_set();
    ASSERT_EQ(6u, s.size());
    s = z.intersect(s);
    ASSERT_EQ(0u, s.size());
    s = b.intersect(s);
    ASSERT_EQ(0u, s.size());
}
