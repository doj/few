/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
* vi: set shiftwidth=4 tabstop=8:
* :indentSize=4:tabSize=8:
*/

#include "gtest/gtest.h"
#include "line.h"

TEST(line_t, handles_empty_string)
{
    line_t l;
    std::string s;
    l.assign(s);
    ASSERT_TRUE(l.empty());
    ASSERT_EQ(std::string(""), l.to_string());
}

TEST(line_t, handles_string_without_nl)
{
    line_t l;
    std::string s = "Dirk";
    l.assign(s);
    ASSERT_FALSE(l.empty());
    ASSERT_EQ(std::string("Dirk"), l.to_string());
}

TEST(line_t, handles_string_with_nl)
{
    line_t l;
    std::string s = "Dirk\n";
    l.assign(s);
    ASSERT_EQ(std::string("Dirk"), l.to_string());
}

TEST(line_t, handles_string_with_crnl)
{
    line_t l;
    std::string s = "Dirk\r\n";
    l.assign(s);
    ASSERT_EQ(std::string("Dirk"), l.to_string());
}

TEST(line_t, handles_string_with_multiple_line_endings)
{
    line_t l;
    std::string s = "Dirk\r\n\r\n\n\n";
    l.assign(s);
    ASSERT_EQ(std::string("Dirk"), l.to_string());
}
