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
    ASSERT_EQ(std::string("dirk"), get_regex_str("/dirk/"));
    ASSERT_EQ(std::string("dirk"), get_regex_str("/dirk/abc"));
    ASSERT_EQ(std::string("a\\/b"), get_regex_str("/a\\/b/i"));
    ASSERT_EQ(std::string("\\/b"), get_regex_str("/\\/b/xdef"));
    ASSERT_EQ(std::string("a\\/"), get_regex_str("/a\\//"));
    ASSERT_EQ(std::string("\\/"), get_regex_str("/\\//"));
    ASSERT_EQ(std::string("a\\\\b"), get_regex_str("/a\\\\b/"));
    ASSERT_EQ(std::string("\\\\b"), get_regex_str("/\\\\b/"));
    ASSERT_EQ(std::string("a\\\\"), get_regex_str("/a\\\\/"));
    ASSERT_EQ(std::string("\\\\"), get_regex_str("/\\\\/!s"));
    ASSERT_EQ(std::string("dirk\\["), get_regex_str("/dirk\\[/"));
    ASSERT_EQ(std::string("dirk[3]+"), get_regex_str("/dirk[3]+/"));
}

TEST(get_regex_str, detects_invalid_regex)
{
    ASSERT_EQ(std::string(""), get_regex_str(""));
    ASSERT_EQ(std::string(""), get_regex_str("a"));
    ASSERT_EQ(std::string(""), get_regex_str("ab"));
    ASSERT_EQ(std::string(""), get_regex_str("//"));
    ASSERT_EQ(std::string(""), get_regex_str("/a\\n"));
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

TEST(is_attr_df, returns_attributes)
{
    std::string r = "|something|bold";
    uint64_t attr; int fg, bg;
    ASSERT_TRUE(is_attr_df(r, attr, fg, bg));
    ASSERT_EQ(A_BOLD, attr);
    ASSERT_EQ(-1, fg);
    ASSERT_EQ(-1, bg);

    r = "|few is|normal";
    ASSERT_TRUE(is_attr_df(r, attr, fg, bg));
    ASSERT_EQ(A_NORMAL, attr);
    ASSERT_EQ(-1, fg);
    ASSERT_EQ(-1, bg);

    r = "|something|dim,standout,green on yellow";
    ASSERT_TRUE(is_attr_df(r, attr, fg, bg));
    ASSERT_EQ(A_DIM|A_STANDOUT, attr);
    ASSERT_EQ(COLOR_GREEN, fg);
    ASSERT_EQ(COLOR_YELLOW, bg);

    r = "|something|italic,underline,bold,reverse";
    ASSERT_TRUE(is_attr_df(r, attr, fg, bg));
    ASSERT_EQ(A_BOLD|A_ITALIC|A_UNDERLINE|A_REVERSE, attr);
    ASSERT_EQ(-1, fg);
    ASSERT_EQ(-1, bg);

    r = "|something|normal,reverse,blink,bold,green on green";
    ASSERT_TRUE(is_attr_df(r, attr, fg, bg));
    ASSERT_EQ(A_NORMAL|A_REVERSE|A_BLINK|A_BOLD, attr);
    ASSERT_EQ(COLOR_GREEN, fg);
    ASSERT_EQ(COLOR_GREEN, bg);

    // don't allow missing attribute
    r = "|something|green on green";
    ASSERT_FALSE(is_attr_df(r, attr, fg, bg));

    // don't allow two color definitions
    r = "|something|bold,green on green,red on blue";
    ASSERT_FALSE(is_attr_df(r, attr, fg, bg));
}

TEST(is_filter_regex, detects_valid_filter_regex)
{
    ASSERT_TRUE(is_filter_regex("/abc/"));
    ASSERT_TRUE(is_filter_regex("/abc/i"));
    ASSERT_TRUE(is_filter_regex("/abc/!"));
    ASSERT_TRUE(is_filter_regex("ab"));
    ASSERT_TRUE(is_filter_regex("!ab"));
    ASSERT_TRUE(is_filter_regex("/a\\/c/"));
    ASSERT_TRUE(is_filter_regex("/a\\//"));
    ASSERT_TRUE(is_filter_regex("/\\//"));
    ASSERT_TRUE(is_filter_regex("/\\\\/"));
    ASSERT_TRUE(is_filter_regex("/Found \\d+ LUNs on target/"));
}

TEST(is_filter_regex, detects_invalid_filter_regex)
{
    ASSERT_FALSE(is_filter_regex("/abc/def/"));
    ASSERT_FALSE(is_filter_regex("|this|bold,red on blue"));
    ASSERT_FALSE(is_filter_regex(""));
    ASSERT_FALSE(is_filter_regex("//"));
}

TEST(parse_replace_df, parses_regular_expressions)
{
    std::string expr = "/a/b/";
    std::string expected_rgx = "a";
    std::string expected_rpl = "b";
    std::string rgx, rpl, err_msg;
    ASSERT_TRUE(parse_replace_df(expr, rgx, rpl, err_msg));
    ASSERT_EQ(expected_rgx, rgx);
    ASSERT_EQ(expected_rpl, rpl);

    expr = "/a*b?d+/this works/";
    expected_rgx = "a*b?d+";
    expected_rpl = "this works";
    ASSERT_TRUE(parse_replace_df(expr, rgx, rpl, err_msg));
    ASSERT_EQ(expected_rgx, rgx);
    ASSERT_EQ(expected_rpl, rpl);

    expr = "/a/b\\/b/";
    expected_rgx = "a";
    expected_rpl = "b\\/b";
    ASSERT_TRUE(parse_replace_df(expr, rgx, rpl, err_msg));
    ASSERT_EQ(expected_rgx, rgx);
    ASSERT_EQ(expected_rpl, rpl);

    expr = "/a/b\\\\b/";
    expected_rgx = "a";
    expected_rpl = "b\\\\b";
    ASSERT_TRUE(parse_replace_df(expr, rgx, rpl, err_msg));
    ASSERT_EQ(expected_rgx, rgx);
    ASSERT_EQ(expected_rpl, rpl);

    expr = "/a\\/c/b/";
    expected_rgx = "a\\/c";
    expected_rpl = "b";
    ASSERT_TRUE(parse_replace_df(expr, rgx, rpl, err_msg));
    ASSERT_EQ(expected_rgx, rgx);
    ASSERT_EQ(expected_rpl, rpl);

    expr = "/a\\\\c/b/";
    expected_rgx = "a\\\\c";
    expected_rpl = "b";
    ASSERT_TRUE(parse_replace_df(expr, rgx, rpl, err_msg));
    ASSERT_EQ(expected_rgx, rgx);
    ASSERT_EQ(expected_rpl, rpl);

    expr = "/\\/a/b/";
    expected_rgx = "\\/a";
    expected_rpl = "b";
    ASSERT_TRUE(parse_replace_df(expr, rgx, rpl, err_msg));
    ASSERT_EQ(expected_rgx, rgx);
    ASSERT_EQ(expected_rpl, rpl);

    expr = "/a\\//b/";
    expected_rgx = "a\\/";
    expected_rpl = "b";
    ASSERT_TRUE(parse_replace_df(expr, rgx, rpl, err_msg));
    ASSERT_EQ(expected_rgx, rgx);
    ASSERT_EQ(expected_rpl, rpl);

    expr = "/\\//b/";
    expected_rgx = "\\/";
    expected_rpl = "b";
    ASSERT_TRUE(parse_replace_df(expr, rgx, rpl, err_msg));
    ASSERT_EQ(expected_rgx, rgx);
    ASSERT_EQ(expected_rpl, rpl);

    expr = "/\\\\/b/";
    expected_rgx = "\\\\";
    expected_rpl = "b";
    ASSERT_TRUE(parse_replace_df(expr, rgx, rpl, err_msg));
    ASSERT_EQ(expected_rgx, rgx);
    ASSERT_EQ(expected_rpl, rpl);

    expr = "/\\\\/\\//";
    expected_rgx = "\\\\";
    expected_rpl = "\\/";
    ASSERT_TRUE(parse_replace_df(expr, rgx, rpl, err_msg));
    ASSERT_EQ(expected_rgx, rgx);
    ASSERT_EQ(expected_rpl, rpl);

    expr = "/\\\\\\//\\/\\\\\\//";
    expected_rgx = "\\\\\\/";
    expected_rpl = "\\/\\\\\\/";
    ASSERT_TRUE(parse_replace_df(expr, rgx, rpl, err_msg));
    ASSERT_EQ(expected_rgx, rgx);
    ASSERT_EQ(expected_rpl, rpl);
}

TEST(parse_replace_df, recognizes_malformed_expressions)
{
    std::string rgx, rpl, err_msg;
    ASSERT_FALSE(parse_replace_df("a/b/", rgx, rpl, err_msg));
    ASSERT_FALSE(parse_replace_df("/a/b", rgx, rpl, err_msg));
    ASSERT_FALSE(parse_replace_df("a/b", rgx, rpl, err_msg));
    ASSERT_FALSE(parse_replace_df("//a/b/", rgx, rpl, err_msg));
    ASSERT_FALSE(parse_replace_df("/a//b/", rgx, rpl, err_msg));
    ASSERT_FALSE(parse_replace_df("/a/b//", rgx, rpl, err_msg));
    ASSERT_FALSE(parse_replace_df("/a/b/c/", rgx, rpl, err_msg));
    ASSERT_FALSE(parse_replace_df("/a/b\\/", rgx, rpl, err_msg));
}
