/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#include "display_info.h"
#include <algorithm>

DisplayInfo&
DisplayInfo::operator= (const lineNum_set_t& s)
{
    displayedLineNum.resize(s.size());
    std::copy(s.begin(), s.end(), displayedLineNum.begin());
    top();
    return *this;
}

bool
DisplayInfo::start()
{
    bottomLineIt = topLineIt;
    return bottomLineIt != displayedLineNum.end();
}

unsigned
DisplayInfo::current() const
{
    return *bottomLineIt;
}

bool
DisplayInfo::next()
{
    if (isLastLineDisplayed()) {
	return false;
    }
    ++bottomLineIt;
    return true;
}

unsigned
DisplayInfo::linesDisplayed() const
{
    return std::distance(topLineIt, bottomLineIt);
}

bool
DisplayInfo::isFirstLineDisplayed() const
{
    return topLineIt == displayedLineNum.begin();
}

bool
DisplayInfo::isLastLineDisplayed() const
{
    displayedLineNum_t::iterator i = bottomLineIt;
    return ++i == displayedLineNum.end();
}

void
DisplayInfo::down()
{
    displayedLineNum_t::iterator it = topLineIt;
    if (++it != displayedLineNum.end()) {
	topLineIt = it;
    }
}

void
DisplayInfo::up()
{
    if (topLineIt != displayedLineNum.begin()) {
	--topLineIt;
    }
}

void
DisplayInfo::top()
{
    topLineIt = displayedLineNum.begin();
}

void
DisplayInfo::page_down()
{
    topLineIt = bottomLineIt;
}
