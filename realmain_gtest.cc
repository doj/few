/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#include "gtest/gtest.h"
#include <sysexits.h>

int realmain(int argc, const char* argv[]);

TEST(realmain, recognizes_invalid_parameters)
{
    ASSERT_EQ(EX_USAGE, realmain(0,NULL));
    ASSERT_EQ(EX_USAGE, realmain(1,NULL));
    const char* argv[] = {
	"fewer",
	NULL
    };
    ASSERT_EQ(EX_USAGE, realmain(0,argv));
    ASSERT_EQ(EX_USAGE, realmain(1,argv));
}

TEST(realmain, recognizes_non_existing_file)
{
    const char* argv[] = {
	"fewer",
	"non existing file",
	NULL
    };
    ASSERT_EQ(EX_NOINPUT, realmain(2,argv));
}
