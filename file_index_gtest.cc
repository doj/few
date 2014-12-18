/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#include "gtest/gtest.h"
#include "file_index.h"
#include <stdexcept>

TEST(file_index, counts_lines_correctly)
{
    file_t f("test.txt");
    file_index f_idx(f);
    f_idx.parse_all();
    ASSERT_EQ(25u, f_idx.lines());
}

TEST(file_index, throws_exception_if_line_index_is_too_large)
{
    file_t f("test.txt");
    file_index f_idx(f);
    ASSERT_THROW(f_idx.line(99999999), std::runtime_error);
}

TEST(file_index, can_get_first_line)
{
    file_t f("test.txt");
    file_index f_idx(f);
    ASSERT_EQ(std::string("This is a test file for the fewer program.\n"), f_idx.line(1).to_string());
}

TEST(file_index, can_get_10th_line)
{
    file_t f("test.txt");
    file_index f_idx(f);
    ASSERT_EQ(std::string("This is line #10.\n"), f_idx.line(10).to_string());
}
