/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#include "gtest/gtest.h"
#include "word_set.h"

TEST(word_set, empty_set_returns_nothing)
{
    std::string err;
    clear_word_set();
    auto s = complete_word_set("", err);
    ASSERT_EQ(0u, s.size());

    s = complete_word_set("something", err);
    ASSERT_EQ(0u, s.size());
}

TEST(word_set, empty_string_completes_everything)
{
    std::string err;
    clear_word_set();
    add_to_word_set("  something everything ");
    auto s = complete_word_set("", err);
    ASSERT_EQ(2u, s.size());
    auto it = s.begin();
    ASSERT_EQ(std::string("everything"), *it);
    ++it;
    ASSERT_EQ(std::string("something"), *it);
    ++it;
    ASSERT_EQ(s.end(), it);
}

TEST(word_set, parsing_works)
{
    std::string err;
    clear_word_set();
    add_to_word_set("a");
    auto s = complete_word_set("", err);
    ASSERT_EQ(1u, s.size());

    clear_word_set();
    add_to_word_set(" a");
    s = complete_word_set("", err);
    ASSERT_EQ(1u, s.size());

    clear_word_set();
    add_to_word_set("a ");
    s = complete_word_set("", err);
    ASSERT_EQ(1u, s.size());

    clear_word_set();
    add_to_word_set(" a ");
    s = complete_word_set("", err);
    ASSERT_EQ(1u, s.size());

    clear_word_set();
    add_to_word_set("a b");
    s = complete_word_set("", err);
    ASSERT_EQ(2u, s.size());

    clear_word_set();
    add_to_word_set(" a b");
    s = complete_word_set("", err);
    ASSERT_EQ(2u, s.size());

    clear_word_set();
    add_to_word_set("a b ");
    s = complete_word_set("", err);
    ASSERT_EQ(2u, s.size());

    clear_word_set();
    add_to_word_set(" a b ");
    s = complete_word_set("", err);
    ASSERT_EQ(2u, s.size());

}

TEST(word_set, completition_works)
{
    std::string err;
    clear_word_set();
    add_to_word_set("dirk oliver jagdmann");
    auto s = complete_word_set("di", err);
    ASSERT_EQ(1u, s.size());
    ASSERT_EQ(std::string("dirk"), *(s.begin()));

    s = complete_word_set("oliver", err);
    ASSERT_EQ(1u, s.size());
    ASSERT_EQ(std::string("oliver"), *(s.begin()));

    s = complete_word_set("Jagd", err);
    ASSERT_EQ(0u, s.size());
}
