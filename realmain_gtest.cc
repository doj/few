/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#include "gtest/gtest.h"
#include <sysexits.h>
#include <getopt.h>

int realmain(int argc, char * const argv[]);

class realmainTest : public ::testing::Test {
protected:
    virtual void SetUp()
    {
	optind = 1;
    }
};

TEST_F(realmainTest, recognizes_invalid_parameters)
{
    ASSERT_EQ(EX_USAGE, realmain(0,NULL));
    ASSERT_EQ(EX_USAGE, realmain(1,NULL));
    const char* const argv[] = {
	"fewer",
	NULL
    };
    ASSERT_EQ(EX_USAGE, realmain(0,const_cast<char * const *>(argv)));
    ASSERT_EQ(EX_USAGE, realmain(1,const_cast<char * const *>(argv)));
}

TEST_F(realmainTest, recognizes_non_existing_file)
{
    const char* const argv[] = {
	"fewer",
	"non existing file",
	NULL
    };
    ASSERT_EQ(EX_NOINPUT, realmain(2,const_cast<char * const *>(argv)));
}

TEST_F(realmainTest, recognizes_tab_width_too_large)
{
    const char* const argv[] = {
	"fewer",
	"--tabwidth",
	"333",
	NULL
    };
    ASSERT_EQ(EX_USAGE, realmain(3,const_cast<char * const *>(argv)));
}

TEST_F(realmainTest, recognizes_tab_width_too_small)
{
    const char* const argv[] = {
	"fewer",
	"--tabwidth",
	"-3",
	NULL
    };
    ASSERT_EQ(EX_USAGE, realmain(3,const_cast<char * const *>(argv)));
}

TEST_F(realmainTest, recognizes_tab_width_0)
{
    const char* const argv[] = {
	"fewer",
	"--tabwidth",
	"0",
	NULL
    };
    ASSERT_EQ(EX_USAGE, realmain(3,const_cast<char * const *>(argv)));
}
