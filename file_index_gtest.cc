/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#include "gtest/gtest.h"
#include "file_index.h"

TEST(file_index, counts_lines_correctly)
{
  file_t f("test.txt");
  file_index f_idx(f);
  ASSERT_EQ(25u, f_idx.lines());
}
