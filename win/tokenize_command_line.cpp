/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#include "../tokenize_command_line.h"
#include "../getenv_str.h"
#include <vector>
#include <iostream>
#include <cstring>
#include <cassert>

bool tokenize_command_line(const std::string& str, int& argc, char** &argv)
{
    if (str.empty()) {
	argc = 0;
	argv = (char**)malloc(sizeof(char*));
	*argv = nullptr;
	return true;
    }

    std::vector<std::string> v;
    enum mode_t {
	NONE,
	SQ, // single quote
	DQ, // double quote
    } m = NONE;
    std::string s;
    int perc = 0; // counts the number of times a percent character was seen
    for (const auto c : str) {
	if (m == NONE) {
	    if (c == ' ' || c == '\t') {
		if (!s.empty()) {
		    v.push_back(s);
		    s.erase();
		}
	    }
	    else if (c == '\'') {
		m = SQ;
	    }
	    else if (c == '"') {
		m = DQ;
	    }
	    else if (c == '%') {
		if (++perc == 2) {
		    perc = 0;
		    std::string::size_type pos = s.rfind('%');
		    assert(pos != std::string::npos);
		    std::string var = s.substr(pos + 1);
		    if (var.empty()) {
			s += c;
		    }
		    else {
			std::string val;
			if (getenv_str(var, val)) {
			    s.erase(pos);
			    s += val;
			}
			else {
			    std::cerr << "environment variable %" << var << "% not found" << std::endl;
			    return false;
			    //s += c;
			}
		    }
		}
		else {
		    s += c;
		}
	    }
	    else {
		s += c;
	    }
	}
	else if (m == SQ) {
	    if (c == '\'') {
		v.push_back(s);
		s.erase();
		m = NONE;
	    }
	    else {
		s += c;
	    }
	}
	else if (m == DQ) {
	    if (c == '"') {
		v.push_back(s);
		s.erase();
		m = NONE;
	    }
	    else {
		s += c;
	    }
	}
	else {
	    std::cerr << "invalid tokenize_command_line() mode: " << m << std::endl;
	    return false;
	}
    }

    if (m != NONE) {
	std::cerr << "unterminated single or double quote: " << str << std::endl;
	return false;
    }

    if (!s.empty()) {
	v.push_back(s);
    }

    argc = v.size();
    argv = (char**)malloc(sizeof(char*) * (argc + 1));
    for (int i = 0; i < argc; ++i) {
	argv[i] = _strdup(v[i].c_str());
    }
    argv[argc] = nullptr;
    return true;
}
