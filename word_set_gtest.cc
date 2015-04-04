/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#include "gtest/gtest.h"
#include "word_set.h"

TEST(word_set, empty_set_returns_nothing)
{
    std::string err, word;
    clear_word_set();
    auto s = complete_word_set(word, err);
    ASSERT_EQ(0u, s.size());

    word = "something";
    s = complete_word_set(word, err);
    ASSERT_EQ(0u, s.size());
}

TEST(word_set, empty_string_completes_everything)
{
    std::string err, word;
    clear_word_set();
    add_to_word_set("  something everything ");
    auto s = complete_word_set(word, err);
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
    std::string err, word;
    clear_word_set();
    add_to_word_set("a");
    auto s = complete_word_set(word, err);
    ASSERT_EQ(1u, s.size());
    ASSERT_EQ(std::string("a"), word);

    clear_word_set();
    add_to_word_set(" a");
    word.clear();
    s = complete_word_set(word, err);
    ASSERT_EQ(1u, s.size());
    ASSERT_EQ(std::string("a"), word);

    clear_word_set();
    add_to_word_set("a ");
    word.clear();
    s = complete_word_set(word, err);
    ASSERT_EQ(1u, s.size());
    ASSERT_EQ(std::string("a"), word);

    clear_word_set();
    add_to_word_set(" a ");
    word.clear();
    s = complete_word_set(word, err);
    ASSERT_EQ(1u, s.size());
    ASSERT_EQ(std::string("a"), word);

    clear_word_set();
    add_to_word_set("a b");
    word.clear();
    s = complete_word_set(word, err);
    ASSERT_EQ(2u, s.size());
    ASSERT_TRUE(word.empty());

    clear_word_set();
    add_to_word_set(" a b");
    word.clear();
    s = complete_word_set(word, err);
    ASSERT_EQ(2u, s.size());
    ASSERT_TRUE(word.empty());

    clear_word_set();
    add_to_word_set("a b ");
    word.clear();
    s = complete_word_set(word, err);
    ASSERT_EQ(2u, s.size());
    ASSERT_TRUE(word.empty());

    clear_word_set();
    add_to_word_set(" a) _b ");
    word.clear();
    s = complete_word_set(word, err);
    ASSERT_EQ(2u, s.size());
    ASSERT_TRUE(word.empty());
}

TEST(word_set, completition_works)
{
    std::string err, word;
    clear_word_set();
    add_to_word_set("dirk oliver jagdmann");
    word = "di";
    auto s = complete_word_set(word, err);
    ASSERT_EQ(1u, s.size());
    ASSERT_EQ(std::string("dirk"), *(s.begin()));
    ASSERT_EQ(std::string("dirk"), word);

    word = "oli";
    s = complete_word_set(word, err);
    ASSERT_EQ(1u, s.size());
    ASSERT_EQ(std::string("oliver"), *(s.begin()));
    ASSERT_EQ(std::string("oliver"), word);

    word = "Jagd";
    s = complete_word_set(word, err);
    ASSERT_EQ(0u, s.size());
    ASSERT_EQ(std::string("Jagd"), word);
}

TEST(word_set, sets_longest_matching_prefix)
{
    std::string err, word;
    clear_word_set();
    add_to_word_set("aaaaa aaab aaaZZ not match aaaaaahg");
    word = "a";
    auto s = complete_word_set(word, err);
    ASSERT_EQ(4u, s.size());
    ASSERT_EQ(std::string("aaa"), word);
}
