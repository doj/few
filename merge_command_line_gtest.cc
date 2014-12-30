/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
* vi: set shiftwidth=4 tabstop=8:
* :indentSize=4:tabSize=8:
*/
#include "gtest/gtest.h"
#include "merge_command_line.h"

TEST(merge_command_line, detects_invalid_parameters)
{
    int argc_left = 1;
    const char*  argv_left_[] = { "l", nullptr };
    const char** argv_left = argv_left_;
    int argc_right = 1;
    const char* argv_right[] = { "r", nullptr };
    argc_left = -1;
    ASSERT_FALSE(merge_command_line_lists(argc_left, argv_left, argc_right, argv_right));
    argc_left = +1;
    const char **null_arr = nullptr;
    ASSERT_FALSE(merge_command_line_lists(argc_left, null_arr,  argc_right, argv_right));
    ASSERT_FALSE(merge_command_line_lists(argc_left, argv_left, -33,        argv_right));
    ASSERT_FALSE(merge_command_line_lists(argc_left, argv_left, argc_right, null_arr));
}

TEST(merge_command_line, handles_empty_right_list)
{
    int argc_left = 1;
    const char*  argv_left_[] = { "l", nullptr };
    const char** argv_left = argv_left_;

    int argc_right = 0;
    const char* argv_right[] = { nullptr };
    ASSERT_TRUE(merge_command_line_lists(argc_left, argv_left, argc_right, argv_right));
    ASSERT_EQ(1, argc_left);
    ASSERT_STREQ("l", argv_left[0]);
}

TEST(merge_command_line, appends_right_list)
{
    int argc_left = 1;
    const char*  argv_left_[] = { "l", nullptr };
    const char** argv_left = argv_left_;

    int argc_right = 2;
    const char* argv_right[] = { "1", "2", nullptr };
    ASSERT_TRUE(merge_command_line_lists(argc_left, argv_left, argc_right, argv_right));
    ASSERT_EQ(3, argc_left);
    ASSERT_STREQ("l", argv_left[0]);
    ASSERT_STREQ("1", argv_left[1]);
    ASSERT_STREQ("2", argv_left[2]);
    ASSERT_EQ(nullptr, argv_left[3]);
}

TEST(merge_command_line, correctly_merges)
{
    int argc_left = 3;
    const char*  argv_left_[] = { "l", "x", "y", nullptr };
    const char** argv_left = argv_left_;

    int argc_right = 2;
    const char* argv_right[] = { "1", "2", nullptr };
    ASSERT_TRUE(merge_command_line_lists(argc_left, argv_left, argc_right, argv_right));
    ASSERT_EQ(5, argc_left);
    ASSERT_STREQ("l", argv_left[0]);
    ASSERT_STREQ("1", argv_left[1]);
    ASSERT_STREQ("2", argv_left[2]);
    ASSERT_STREQ("x", argv_left[3]);
    ASSERT_STREQ("y", argv_left[4]);
    ASSERT_EQ(nullptr, argv_left[5]);
}
