/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#include "progress_functor.h"
#include "curses_attr.h"

CursesProgressFunctor::~CursesProgressFunctor()
{
    if (max_len_ > 0) {
	std::string s(max_len_, ' ');
	curses_attr a(attr_);
	mvprintw(y_, x_, "%s", s.c_str());
	refresh();
    }
}

void
CursesProgressFunctor::progress(unsigned num, unsigned perc)
{
    std::string s = desc_ + std::to_string(num) + ' ' + std::to_string(perc) + "% ";
    if (s.size() > max_len_) {
	max_len_ = s.size();
    }
    {
	curses_attr a(attr_);
	mvprintw(y_, x_, "%s", s.c_str());
    }
    refresh();
}
