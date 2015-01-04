/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#include "gtest/gtest.h"
#include "normalize_regex.h"
#include "curses_attr.h"

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

TEST(get_regex_flags, works)
{
    ASSERT_EQ(std::string(""), get_regex_flags(""));
    ASSERT_EQ(std::string(""), get_regex_flags("/dirk/"));
    ASSERT_EQ(std::string("cba"), get_regex_flags("/dirk/abc"));
}

TEST(get_regex_str, works)
{
    ASSERT_EQ(std::string(""), get_regex_str(""));
    ASSERT_EQ(std::string("dirk"), get_regex_str("/dirk/"));
    ASSERT_EQ(std::string("dirk"), get_regex_str("/dirk/abc"));
}

TEST(normalize_regex, detects_df_attr_syntax)
{
    std::string r = "|something|bold";
    ASSERT_EQ(r, normalize_regex(r));
    r = "|something|bold,italic,underline";
    ASSERT_EQ(r, normalize_regex(r));
    r = "|something|dim,standout,green on yellow";
    ASSERT_EQ(r, normalize_regex(r));
}

TEST(is_df_with_curses_attr, returns_attributes)
{
    std::string r = "|something|bold";
    uint64_t attr; int fg, bg;
    ASSERT_EQ(true, is_df_with_curses_attr(r, attr, fg, bg));
    ASSERT_EQ(A_BOLD, attr);
    ASSERT_EQ(-1, fg);
    ASSERT_EQ(-1, bg);

    r = "|few is|normal";
    ASSERT_EQ(true, is_df_with_curses_attr(r, attr, fg, bg));
    ASSERT_EQ(A_NORMAL, attr);
    ASSERT_EQ(-1, fg);
    ASSERT_EQ(-1, bg);

    r = "|something|dim,standout,green on yellow";
    ASSERT_EQ(true, is_df_with_curses_attr(r, attr, fg, bg));
    ASSERT_EQ(A_DIM|A_STANDOUT, attr);
    ASSERT_EQ(COLOR_GREEN, fg);
    ASSERT_EQ(COLOR_YELLOW, bg);

    // still failing, the regular expression does not provide matches for all attributes but only the first and last attribute
    r = "|something|italic,underline,bold,reverse";
    ASSERT_EQ(true, is_df_with_curses_attr(r, attr, fg, bg));
    ASSERT_EQ(A_BOLD|A_ITALIC|A_UNDERLINE|A_REVERSE, attr);
    ASSERT_EQ(-1, fg);
    ASSERT_EQ(-1, bg);
}
