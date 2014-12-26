/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#pragma once

#define _XOPEN_SOURCE_EXTENDED 1
#if defined(__FreeBSD__)
#include <ncurses.h>
#else
/// define to enable curses with wide characters
#include <ncursesw/curses.h>
#endif

/**
 * helper class to manage curses attributes.
 * The constructor sets an attribute, the destructor unsets it.
 */
class curses_attr
{
    attr_t attrs_;
    short pair_;
public:
    explicit curses_attr(unsigned a)
    {
	attr_get(&attrs_, &pair_, nullptr);
	attron(a);
    }
    ~curses_attr()
    {
	attrset(attrs_);
	color_set(pair_, nullptr);
    }
};
