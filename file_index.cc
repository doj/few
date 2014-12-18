/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#include "file_index.h"
#include <stdexcept>
#include <cassert>

file_index::file_index(const file_t& f) :
    file_(f)
{
    // we start counting lines with number 1. So add an invalid pointer to index 0.
    line_.push_back(line_t(nullptr, nullptr, 0));
}

bool file_index::parse_line(const unsigned index)
{
    //asm("int3");

    if (file_.empty()) {
	return false;
    }

    if (index <= lines()) {
	return true;
    }

    unsigned num = 0;
    file_t::const_iterator it = nullptr;
    if (index > 1 && lines() > 0) {
	line_t current_line = line_[lines()];
	it = current_line.end_;
	num = current_line.num_;
    } else {
	it = file_.begin();
    }

    if (it == file_.end()) {
	return false;
    }

    file_t::const_iterator beg = it;
    file_t::const_iterator end = nullptr;
    while(num < index) {
	if (*it == '\n') {
	    end = it + 1;
	    line_.push_back(line_t(beg, end, ++num));
	    beg = end;
	}
	++it;
	if (it == file_.end()) {
	    if (it != beg) {
		end = it;
		line_.push_back(line_t(beg, end, ++num));
	    }
	    break;
	}
    }

    return num == index;
}

/// \todo remove this function once we have a better compiler!
#include <sstream>
namespace std {
    template <typename T>
    std::string to_string(const T& t)
    {
	std::ostringstream os;
	os << t;
	return os.str();
    }
}

line_t file_index::line(const unsigned idx)
{
    if (idx > lines()) {
	if (! parse_line(idx)) {
	    throw std::runtime_error("file_index::line(" + std::to_string(idx) + "): index too large, file only contains " + std::to_string(lines()));
	}
    }
    return line_[idx];
}
