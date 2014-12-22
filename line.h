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
    /// first character of the line.
    const char *beg_;
    /// one past the last character of the line excluding NL and CR characters.
    const char *end_;
    /// first character of next line, can be nullptr.
    const char *next_;
    /// line number.
    /*const*/ unsigned num_;

    line_t(const char *begin, const char *end, const char *next, const unsigned num) :
	beg_(begin),
	end_(end),
	next_(next),
	num_(num)
    {
	assert(beg_ <= end_);
	if (next_) {
	    assert(beg_ < next_);
	    assert(end_ < next_);
	}
    }

    std::string to_string() const
    {
	return std::string(beg_, end_);
    }

    bool empty() const { return beg_ == end_; }

    line_t& assign(const std::string& s)
    {
	beg_ = s.data();
	end_ = s.data() + s.size();
	return *this;
    }
};
