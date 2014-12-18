/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#pragma once
#include <string>
#include <cassert>

class line_t
{
public:
    const char *beg_;
    const char *end_;
    /*const*/ unsigned num_;

    line_t(const char *begin, const char *end, const unsigned num) :
	beg_(begin),
	end_(end),
	num_(num)
    {
	assert(beg_ <= end_);
    }

    std::string to_string() const
    {
	return std::string(beg_, end_);
    }

};
