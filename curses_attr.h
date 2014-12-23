/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#pragma once
#include <ncurses.h>

/**
 * helper class to manage curses attributes.
 * The constructor sets an attribute, the destructor unsets it.
 */
class curses_attr
{
    const unsigned a_;
public:
    explicit curses_attr(unsigned a) : a_(a) { attron(a_); }
    ~curses_attr() { attroff(a_); }
};
