/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#pragma once

/// define to enable curses with wide characters
#define _XOPEN_SOURCE_EXTENDED 1
#include <ncursesw/curses.h>

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
