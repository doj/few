/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#include "gtest/gtest.h"
#include "complete_filename.h"

TEST(complete_filename, matches_current_directory_for_empty_string)
{
    std::string err, path;
    auto s = complete_filename(path, err);
    ASSERT_GT(s.size(), 10u);
    ASSERT_EQ(0u, err.size());
}

TEST(complete_filename, matches_complete_relative_files)
{
    std::string err, path;
    path = "README.md";
    auto s = complete_filename(path, err);
    ASSERT_EQ(1u, s.size());
    ASSERT_EQ(0u, err.size());

    path = "README.m";
    s = complete_filename(path, err);
    ASSERT_EQ(1u, s.size());
    ASSERT_EQ(0u, err.size());
    ASSERT_EQ(std::string("README.md"), path);

    path = "unix/complete_filename.cc";
    s = complete_filename(path, err);
    ASSERT_EQ(1u, s.size());
    ASSERT_EQ(0u, err.size());
}

TEST(complete_filename, matches_complete_absolute_files)
{
    std::string err, path;
    path = "/etc/fstab";
    auto s = complete_filename(path, err);
    ASSERT_EQ(1u, s.size());
    ASSERT_EQ(0u, err.size());
}

TEST(complete_filename, matches_absolute_pathnames)
{
    std::string err, path;
    path = "/etc/cron";
    auto s = complete_filename(path, err);
    ASSERT_GT(s.size(), 2u);
    ASSERT_EQ(0u, err.size());

    path = "/tmp";
    s = complete_filename(path, err);
    ASSERT_EQ(1u, s.size());
    ASSERT_EQ(0u, err.size());
    ASSERT_EQ(std::string("/tmp/"), *(s.begin()));
}

TEST(complete_filename, matches_relative_pathnames)
{
    std::string err, path;
    path = "win/ge";
    auto s = complete_filename(path, err);
    ASSERT_GT(s.size(), 5u);
    ASSERT_EQ(0u, err.size());
}
