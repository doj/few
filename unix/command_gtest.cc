/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#include "gtest/gtest.h"
#include "command.h"

TEST(Command, can_detect_success)
{
    std::string msg;
    ASSERT_TRUE(run_command("true", msg));
}

TEST(Command, can_detect_failure)
{
    std::string msg;
    ASSERT_FALSE(run_command("false", msg));
}

TEST(Command, detects_invalid_command_as_failure)
{
    std::string msg;
    ASSERT_FALSE(run_command("this_is_a_failing_command", msg));
}

TEST(Command, can_run_background_program)
{
    std::string msg;
    pid_t pid = run_command_background("true", msg);
    ASSERT_GT(pid, 0);
    int status = ~0;
    ASSERT_EQ(pid, waitpid(pid, &status, 0));
    ASSERT_TRUE(WIFEXITED(status));
    ASSERT_EQ(EXIT_SUCCESS, WEXITSTATUS(status));
}

TEST(Command, can_run_failing_background_program)
{
    std::string msg;
    pid_t pid = run_command_background("false", msg);
    ASSERT_GT(pid, 0);
    int status = ~0;
    ASSERT_EQ(pid, waitpid(pid, &status, 0));
    ASSERT_TRUE(WIFEXITED(status));
    ASSERT_GT(WEXITSTATUS(status), 0);
}

TEST(Command, can_run_non_existent_background_program)
{
    std::string msg;
    pid_t pid = run_command_background("this program does not exist", msg);
    ASSERT_GT(pid, 0);
    int status = ~0;
    ASSERT_EQ(pid, waitpid(pid, &status, 0));
    ASSERT_TRUE(WIFEXITED(status));
    ASSERT_GT(WEXITSTATUS(status), 0);
}
