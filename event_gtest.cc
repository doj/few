/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#include "gtest/gtest.h"
#include "event.h"

TEST(event, empty_queue)
{
    ASSERT_FALSE(eventPending());
    ASSERT_THROW(eventGet(), std::runtime_error);
}

TEST(event, add_and_remove_events)
{
    ASSERT_FALSE(eventPending());
    event expected1("ex1");
    eventAdd(expected1);
    event expected2("ex2");
    eventAdd(expected2);

    ASSERT_TRUE(eventPending());
    ASSERT_EQ(expected1, eventGet());
    ASSERT_EQ(expected2, eventGet());
    ASSERT_FALSE(eventPending());
}
