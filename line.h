/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#pragma once
#include <string>
#include <cassert>
#include <iostream>
#include "types.h"

/**
 * The line_t object manages pointer into a string.
 */
class line_t
{
    void strip()
    {
	while (beg_ < end_) {
	    const char c = *(end_ - 1);
	    if (c == '\n' || c == '\r') {
		--end_;
	    } else {
		break;
	    }
	}
    }

public:
    /// first character of the line.
    const char *beg_;
    /// one past the last character of the line excluding NL and CR characters.
    const char *end_;
    /// first character of next line, can be nullptr.
    const char *next_;
    /// line number.
    /*const*/ line_number_t num_;

    line_t() :
	beg_(nullptr),
	end_(nullptr),
	next_(nullptr),
	num_(0)
    {}

    line_t(const char *begin, const char *end, const char *next, const line_number_t num) :
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
	strip();
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
	strip();
	return *this;
    }
};

inline std::ostream&
operator<< (std::ostream& os, const line_t& l)
{
    // \todo maybe there's a better way to print a pointer range to an output stream?
    return os << l.to_string();
}
