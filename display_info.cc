/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#include "display_info.h"
#include <algorithm>
#include <cassert>

void
DisplayInfo::go_to_approx(const line_number_t line_num)
{
    if (line_num == 0 || displayedLineNum.empty()) {
	top();
    } else {
	topLineIt = std::lower_bound(displayedLineNum.begin(), displayedLineNum.end(), line_num);
	if (topLineIt == displayedLineNum.begin()) {
	    // do nothing
	} else if (topLineIt == displayedLineNum.end()) {
	    --topLineIt;
	} else if (*topLineIt == line_num) {
	    // do nothing
	} else {
	    --topLineIt;
	}
    }
}

void
DisplayInfo::assign(lineNum_vector_t&& v)
{
    unsigned old_line_num = 0;
    if (topLineIt != displayedLineNum.end()) {
	old_line_num = *topLineIt;
    }

    displayedLineNum = std::move(v);

    go_to_approx(old_line_num);
}

bool
DisplayInfo::start()
{
    bottomLineIt = topLineIt;
    return bottomLineIt != displayedLineNum.end();
}

line_number_t
DisplayInfo::current() const
{
    if (bottomLineIt == displayedLineNum.end()) {
	return 0;
    }
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
    auto it = topLineIt;
    if (it == displayedLineNum.end()) {
	return;
    }
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

line_number_t
DisplayInfo::bottomLineNum() const
{
    if (bottomLineIt == displayedLineNum.end()) {
	return 0;
    }
    return *bottomLineIt;
}

std::string
DisplayInfo::info() const
{
    return "top #" + std::to_string(*topLineIt) + " bottom #" + std::to_string(*bottomLineIt);
}

line_number_t
DisplayInfo::lastLineNum() const
{
    if (displayedLineNum.size() == 0) {
	return 0;
    }
    return displayedLineNum[displayedLineNum.size() - 1];
}

bool
DisplayInfo::go_to(const line_number_t lineNum)
{
    displayedLineNum_t::iterator i = std::find(displayedLineNum.begin(), displayedLineNum.end(), lineNum);
    if (i == displayedLineNum.end()) {
	return false;
    }
    bottomLineIt = topLineIt = i;
    return true;
}

void
DisplayInfo::go_to_perc(unsigned p)
{
    if (p == 0) {
	top();
	return;
    }
    if (p > 100) {
	p = 100;
    }
    go_to_approx(static_cast<uint64_t>(lastLineNum()) * static_cast<uint64_t>(p) / static_cast<uint64_t>(100u));
}

line_number_t
DisplayInfo::topLineNum() const
{
    if (topLineIt == displayedLineNum.end()) {
	return 0;
    }
    return *topLineIt;
}
