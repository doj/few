/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#include "gtest/gtest.h"
#include "file_index.h"

TEST(regex_index, does_filter_non_empty_lines)
{
    auto fi = std::make_shared<file_index>("test.txt");
    auto ri = std::make_shared<regex_index>("\\w");
    fi->parse_all(ri);
    ASSERT_GE(ri->size(), 3u);
}

TEST(regex_index, i_flag)
{
    auto fi = std::make_shared<file_index>("test.txt");
    auto ri = std::make_shared<regex_index>("/this is line #10./i");
    fi->parse_all(ri);
    ASSERT_EQ(1u, ri->size());
}

TEST(regex_index, not_flag)
{
    auto fi = std::make_shared<file_index>("test.txt");
    auto ri = std::make_shared<regex_index>("/#/!");
    fi->parse_all(ri);
    ASSERT_GE(ri->size(), 3u);
}

TEST(regex_index, intersect_works)
{
    auto fi = std::make_shared<file_index>("test.txt");

    auto a = std::make_shared<regex_index>("/#/!");
    fi->parse_all(a);

    auto b = std::make_shared<regex_index>("contains");
    fi->parse_all(b);

    auto s = b->intersect(a->lineNum_set());
    ASSERT_EQ(1u, s.size());

    auto c = std::make_shared<regex_index>("this does not match anything");
    fi->parse_all(c);
    ASSERT_EQ(0u, c->size());
    s = c->intersect(a->lineNum_set());
    ASSERT_EQ(0u, s.size());
}

TEST(regex_index, intersect_works_on_test2_txt)
{
    auto fi = std::make_shared<file_index>("test2.txt");
    fi->parse_all();
    ASSERT_EQ(6u, fi->size());

    auto word_characters = std::make_shared<regex_index>("\\w");
    fi->parse_all(word_characters);
    ASSERT_EQ(6u, word_characters->size());

    auto b = std::make_shared<regex_index>("b+");
    fi->parse_all(b);
    ASSERT_EQ(1u, b->size());

    auto s = fi->lineNum_set();
    ASSERT_EQ(6u, s.size());
    s = word_characters->intersect(s);
    ASSERT_EQ(6u, s.size());
    s = b->intersect(s);
    ASSERT_EQ(1u, s.size());

    s = fi->lineNum_set();
    ASSERT_EQ(6u, s.size());
    s = b->intersect(s);
    ASSERT_EQ(1u, s.size());
    s = word_characters->intersect(s);
    ASSERT_EQ(1u, s.size());

    auto z = std::make_shared<regex_index>("z");
    ASSERT_EQ(0u, z->size());
    s = fi->lineNum_set();
    ASSERT_EQ(6u, s.size());
    s = z->intersect(s);
    ASSERT_EQ(0u, s.size());
    s = b->intersect(s);
    ASSERT_EQ(0u, s.size());
}
