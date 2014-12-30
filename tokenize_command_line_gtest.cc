/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
* vi: set shiftwidth=4 tabstop=8:
* :indentSize=4:tabSize=8:
*/
#include "gtest/gtest.h"
#include "tokenize_command_line.h"

TEST(tokenize_command_line, handles_empty_string)
{
    int argc;
    char **argv;
    ASSERT_TRUE(tokenize_command_line("", argc, argv));
    ASSERT_EQ(0, argc);
    ASSERT_TRUE(argv != nullptr);
    ASSERT_TRUE(*argv == nullptr);
    tokenize_command_line_free(argv);
}

TEST(tokenize_command_line, handles_one_parameter)
{
    int argc;
    char **argv;
    ASSERT_TRUE(tokenize_command_line("a", argc, argv));
    ASSERT_EQ(1, argc);
    ASSERT_TRUE(argv != nullptr);
    ASSERT_STREQ("a", argv[0]);
    ASSERT_TRUE(argv[1] == nullptr);
    tokenize_command_line_free(argv);
}

TEST(tokenize_command_line, handles_two_parameters)
{
    int argc;
    char **argv;
    ASSERT_TRUE(tokenize_command_line("a b", argc, argv));
    ASSERT_EQ(2, argc);
    ASSERT_STREQ("a", argv[0]);
    ASSERT_STREQ("b", argv[1]);
    ASSERT_TRUE(argv[2] == nullptr);
    tokenize_command_line_free(argv);
}

TEST(tokenize_command_line, handles_tab_as_whitespace)
{
    int argc;
    char **argv;
    ASSERT_TRUE(tokenize_command_line("a\tb", argc, argv));
    ASSERT_EQ(2, argc);
    ASSERT_STREQ("a", argv[0]);
    ASSERT_STREQ("b", argv[1]);
    ASSERT_TRUE(argv[2] == nullptr);
    tokenize_command_line_free(argv);
}

TEST(tokenize_command_line, handles_single_quotes)
{
    int argc;
    char **argv;
    ASSERT_TRUE(tokenize_command_line("'a' 'b c d'", argc, argv));
    ASSERT_EQ(2, argc);
    ASSERT_STREQ("a", argv[0]);
    ASSERT_STREQ("b c d", argv[1]);
    ASSERT_TRUE(argv[2] == nullptr);
    tokenize_command_line_free(argv);
}

TEST(tokenize_command_line, handles_double_quotes)
{
    int argc;
    char **argv;
    ASSERT_TRUE(tokenize_command_line("\"a\" \"b c d\"", argc, argv));
    ASSERT_EQ(2, argc);
    ASSERT_STREQ("a", argv[0]);
    ASSERT_STREQ("b c d", argv[1]);
    ASSERT_TRUE(argv[2] == nullptr);
    tokenize_command_line_free(argv);
}