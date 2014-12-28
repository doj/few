/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#include "progress_functor.h"
#include "curses_attr.h"
#include "getRSS.h"
#include "types.h"

extern unsigned verbose;

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
    if (verbose) {
	s += " curMB " +std::to_string(getCurrentRSS()/1024/1024);
    }
    if (s.size() > max_len_) {
	max_len_ = s.size();
    }
    {
	curses_attr a(attr_);
	mvprintw(y_, x_, "%s", s.c_str());
    }
    refresh();
}

OStreamProgressFunctor::~OStreamProgressFunctor() { os_ << std::endl; }

void
OStreamProgressFunctor::progress(unsigned num, unsigned perc)
{
    os_ << "\r" << desc_ << num << ' ' << perc << "%";
    if (verbose) {
	os_ << " curMB " << getCurrentRSS()/1024/1024 << " peakMB " << getPeakRSS()/1024/1024;
    }
    os_ << std::flush;
}
