/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#pragma once
#include <stdexcept>

class error : public std::runtime_error
{
    const int exit_status_;
public:
    error(const std::string& msg, int exit_status) :
	std::runtime_error(msg),
	exit_status_(exit_status)
    {}
    int exit_status() const { return exit_status_; }
};
