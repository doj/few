/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#pragma once
#include <string>

class line_t
{
    const char *begin_;
    const char *end_;

public:
    line_t(const char *begin, const char *end) :
	begin_(begin),
	end_(end)
    {}

    std::string to_string() const
    {
	return std::string(begin_, end_);
    }

};
