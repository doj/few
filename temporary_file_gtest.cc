/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#include "gtest/gtest.h"
#include "temporary_file.h"
#include "to_wide.h"
#include "errno_str.h"

TEST(TemporaryFile, can_create_a_file)
{
    TemporaryFile tmp;
    FILE *f = tmp.file();
    ASSERT_TRUE(f != nullptr);
    const char *expected = "A write test!\r\n\n";
    const unsigned expected_len = strlen(expected);
    auto res = fwrite(expected, expected_len, 1, f);
    if (res == 0) {
	std::cerr << "could not fwrite: " << errno_str() << " to " << tmp.filename() << std::endl;
    }
    ASSERT_EQ(1u, res);
    fclose(f);

    f = fopen(to_utf8(tmp.filename()).c_str(), "rb");
    ASSERT_TRUE(f != nullptr);
    char buf[100];
    ASSERT_LT(expected_len, sizeof(buf));
    res = fread(buf, 1, sizeof(buf), f);
    if (res == 0) {
	std::cerr << "could not fread: " << errno_str() << " from " << tmp.filename() << std::endl;
    }
    ASSERT_EQ(expected_len, res);
    buf[expected_len] = 0;
    fclose(f);
    ASSERT_STREQ(expected, buf);
}
