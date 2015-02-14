/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#pragma once
#include <stdint.h>

#if defined(_WIN32)
#include <windows.h>
#undef MOUSE_MOVED // this is also defined by PDCurses

#define PDC_WIDE 1
#define NCURSES_MOUSE_VERSION 1
#include "win/curses.h"

#elif defined(__FreeBSD__)
#define _XOPEN_SOURCE_EXTENDED 1
#include <ncurses.h>
#else
#define _XOPEN_SOURCE_EXTENDED 1
/// define to enable curses with wide characters
#include <ncursesw/curses.h>
#endif

#ifndef A_ITALIC
#define A_ITALIC 0
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
    explicit curses_attr(uint64_t a)
    {
	attr_get(&attrs_, &pair_, nullptr);
	attron(static_cast<chtype>(a));
    }
    ~curses_attr()
    {
	attrset(attrs_);
	color_set(pair_, nullptr);
    }
};
