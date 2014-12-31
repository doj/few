/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#include "gtest/gtest.h"
#include <sysexits.h>
#include <getopt.h>
#include <cstdlib>

#if defined(_WIN32)
void setenv(const char *key, const char* val, int)
{
    _putenv_s(key, val);
}
#endif

int realmain(int argc, char * const argv[]);

static const char* FEWOPTIONS = "FEWOPTIONS";

class realmainTest : public ::testing::Test {
protected:
    virtual void SetUp()
    {
	optind = 1;
    }
    virtual void TearDown()
    {
#if defined(_WIN32)
	_putenv_s(FEWOPTIONS, "");
#else
	unsetenv(FEWOPTIONS);
#endif
    }
};

TEST_F(realmainTest, recognizes_invalid_parameters)
{
    ASSERT_EQ(EX_USAGE, realmain(0,NULL));
    ASSERT_EQ(EX_USAGE, realmain(1,NULL));
    const char* const argv[] = {
	"few",
	NULL
    };
    ASSERT_EQ(EX_USAGE, realmain(0,const_cast<char * const *>(argv)));
}

TEST_F(realmainTest, recognizes_non_existing_file)
{
    const char* const argv[] = {
	"few",
	"non existing file",
	NULL
    };
    ASSERT_EQ(EX_NOINPUT, realmain(2,const_cast<char * const *>(argv)));
}

TEST_F(realmainTest, recognizes_tab_width_too_large)
{
    const char* const argv[] = {
	"few",
	"--tabwidth",
	"333",
	NULL
    };
    ASSERT_EQ(EX_USAGE, realmain(3,const_cast<char * const *>(argv)));
}

TEST_F(realmainTest, recognizes_tab_width_too_small)
{
    const char* const argv[] = {
	"few",
	"--tabwidth",
	"-3",
	NULL
    };
    ASSERT_EQ(EX_USAGE, realmain(3,const_cast<char * const *>(argv)));
}

TEST_F(realmainTest, recognizes_tab_width_0)
{
    const char* const argv[] = {
	"few",
	"--tabwidth",
	"0",
	NULL
    };
    ASSERT_EQ(EX_USAGE, realmain(3,const_cast<char * const *>(argv)));
}

TEST_F(realmainTest, recognizes_too_many_regex)
{
    const char* const argv[] = {
	"few",
	"--regex", "a",
	"--regex", "a",
	"--regex", "a",
	"--regex", "a",
	"--regex", "a",
	"--regex", "a",
	"--regex", "a",
	"--regex", "a",
	"--regex", "a",
	"--regex", "a",
	"--regex", "a",
	NULL
    };
    ASSERT_EQ(EX_USAGE, realmain(1 + 11*2,const_cast<char * const *>(argv)));
}

TEST_F(realmainTest, recognizes_failure_to_parse_FEWOPTIONS_with_missing_quote)
{
    const char* const argv[] = {
	"few",
	"non existing file",
	NULL
    };
    setenv(FEWOPTIONS, "missing single 'quote", 1);
    ASSERT_EQ(EX_USAGE, realmain(2,const_cast<char * const *>(argv)));

    setenv(FEWOPTIONS, "missing double \" quote", 1);
    ASSERT_EQ(EX_USAGE, realmain(2,const_cast<char * const *>(argv)));
}

TEST_F(realmainTest, recognizes_failure_to_parse_FEWOPTIONS_with_unset_variable)
{
    const char* const argv[] = {
	"few",
	"non existing file",
	NULL
    };
#if defined(_WIN32)
    setenv(FEWOPTIONS, "first %NOT_EXISTING_VARIBLE% second", 1);
#else
    setenv(FEWOPTIONS, "first $NOT_EXISTING_VARIBLE second", 1);
#endif
    ASSERT_EQ(EX_USAGE, realmain(2, const_cast<char * const *>(argv)));
}
