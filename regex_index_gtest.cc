/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#include "gtest/gtest.h"
#include "regex_index.h"

TEST(regex_index, does_filter_non_empty_lines)
{
    file_t f("test.txt");
    file_index f_idx(f);
    regex_index r_idx(f_idx, "\\S+");
    //ASSERT_GT(3, r_idx.lines());
}
