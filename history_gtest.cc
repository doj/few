/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#include "gtest/gtest.h"
#include "history.h"
#if defined(_WIN32)
#include <io.h>
#include <stdio.h>
namespace {
    int unlink(const std::string& fn)
    {
	return _unlink(fn.c_str());
    }
    int unlink(const std::wstring& fn)
    {
	return _wunlink(fn.c_str());
    }
}
#else
#include <unistd.h>
#endif

TEST(History, can_cycle_through_history)
{
    auto h = std::make_shared<History>();
    const std::string eins = "1";
    h->add(eins);
    const std::string zwei = "2";
    h->add(zwei);
    const std::string drei = "3";
    h->add(drei);

    const std::string c = "c";
    auto i = h->begin(c);
    ASSERT_TRUE(i->atEnd());
    ASSERT_EQ(c, i->next());
    ASSERT_EQ(c, i->next());
    ASSERT_TRUE(i->atEnd());

    ASSERT_EQ(drei, i->prev());
    ASSERT_FALSE(i->atEnd());
    ASSERT_EQ(zwei, i->prev());
    ASSERT_EQ(eins, i->prev());
    ASSERT_EQ(eins, i->prev());
    ASSERT_EQ(eins, i->prev());
    ASSERT_FALSE(i->atEnd());

    ASSERT_EQ(zwei, i->next());
    ASSERT_EQ(drei, i->next());
    ASSERT_EQ(c, i->next());
    ASSERT_TRUE(i->atEnd());
    ASSERT_EQ(c, i->next());
    ASSERT_TRUE(i->atEnd());
}

TEST(History, can_save_and_load)
{
    const std::string filename = "history.test";
    unlink(filename.c_str());

    auto h = std::make_shared<History>(filename);
    const std::string eins = "1";
    h->add(eins);
    const std::string zwei = "2";
    h->add(zwei);
    const std::string drei = "3";
    h->add(drei);
    h = nullptr;

    h = std::make_shared<History>(filename);
    auto i = h->begin("");
    ASSERT_EQ(drei, i->prev());
    ASSERT_EQ(zwei, i->prev());
    ASSERT_EQ(eins, i->prev());
    auto fn = h->filename();
    h = nullptr;
    i = nullptr;

    ASSERT_EQ(0, unlink(fn.c_str()));
}

TEST(History, empty_history_only_returns_initial_iterator_value)
{
    auto h = std::make_shared<History>();
    const std::string s = "s";
    auto i = h->begin(s);
    ASSERT_EQ(s, i->prev());
    ASSERT_EQ(s, i->prev());
    ASSERT_EQ(s, i->next());
    ASSERT_EQ(s, i->prev());
    ASSERT_TRUE(i->atEnd());
}
