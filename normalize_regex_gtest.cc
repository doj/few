/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#include "gtest/gtest.h"
#include "normalize_regex.h"

TEST(normalize_regex, empty_string)
{
    ASSERT_EQ(std::string(""), normalize_regex(""));
}

TEST(normalize_regex, does_not_change_regular_form)
{
    std::string s = "/a/";
    ASSERT_EQ(s, normalize_regex(s));
    s = "/abc*\\d+/";
    ASSERT_EQ(s, normalize_regex(s));
    s = "/ab(gh)/!i";
    ASSERT_EQ(s, normalize_regex(s));
    s = "/\\//";
    ASSERT_EQ(s, normalize_regex(s));
}

TEST(normalize_regex, converts_short_form_to_regular_form)
{
    std::string s = "a";
    std::string expected = "/" + s + "/";
    ASSERT_EQ(expected, normalize_regex(s));
    s = "abc*\\d+";
    expected = "/" + s + "/";
    ASSERT_EQ(expected, normalize_regex(s));

    s = "/";
    expected = "///";
    ASSERT_EQ(expected, normalize_regex(s));
}

TEST(normalize_regex, converts_short_negative_match_form_to_regular_form)
{
    std::string s = "!a";
    std::string expected = "/a/!";
    ASSERT_EQ(expected, normalize_regex(s));

    s = "!";
    expected = "/!/";
    ASSERT_EQ(expected, normalize_regex(s));
}

TEST(normalize_regex, recognizes_missing_slash_as_short_form)
{
    std::string s = "/a/bcd";
    std::string expected = "//a/bcd/";
    ASSERT_EQ(expected, normalize_regex(s));
}
