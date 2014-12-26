/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#include "gtest/gtest.h"
#include "file_index.h"
#include "intersect.h"
#include <iterator>

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

    lineNum_vector_intersect_vector_t v = { std::make_pair(a->lineNum_vector().begin(), a->lineNum_vector().end()),
					    std::make_pair(b->lineNum_vector().begin(), b->lineNum_vector().end()) };
    lineNum_vector_t s;
    ASSERT_EQ(1u, multiple_set_intersect(v.begin(), v.end(), std::back_insert_iterator<lineNum_vector_t>(s)));
    ASSERT_EQ(1u, s.size());

    auto c = std::make_shared<regex_index>("this does not match anything");
    fi->parse_all(c);
    ASSERT_EQ(0u, c->size());

    v[1] = std::make_pair(c->lineNum_vector().begin(), c->lineNum_vector().end());
    s.clear();
    ASSERT_EQ(0u, multiple_set_intersect(v.begin(), v.end(), std::back_insert_iterator<lineNum_vector_t>(s)));
    ASSERT_EQ(0u, s.size());
}
