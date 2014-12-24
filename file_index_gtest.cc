/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#include "gtest/gtest.h"
#include "file_index.h"
#include <stdexcept>
#include <memory>

TEST(file_index, counts_lines_correctly)
{
    auto f_idx = std::make_shared<file_index>("test.txt");
    f_idx->parse_all();
    ASSERT_EQ(25u, f_idx->size());
}

TEST(file_index, throws_exception_if_line_index_is_too_large)
{
    auto f_idx = std::make_shared<file_index>("test.txt");
    ASSERT_THROW(f_idx->line(99999999), std::runtime_error);
}

TEST(file_index, can_get_first_line)
{
    auto f_idx = std::make_shared<file_index>("test.txt");
    ASSERT_EQ(std::string("This is a test file for the few program."), f_idx->line(1).to_string());
}

TEST(file_index, can_get_10th_line)
{
    auto f_idx = std::make_shared<file_index>("test.txt");
    ASSERT_EQ(std::string("This is line #10."), f_idx->line(10).to_string());
}
