/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#include "normalize_regex.h"
#include "curses_attr.h"
#include <regex>
#include <map>
#include <cassert>

std::string normalize_regex(std::string regex)
{
    // check for some simple forms
    if (regex.empty()) {
	return regex;
    }

    if (regex == "/") {
	return "///";
    }

    if (regex == "!") {
	return "/!/";
    }

    // check for display filter with curses attributes
    uint64_t attr; int fg, bg;
    if (is_df_with_curses_attr(regex, attr, fg, bg)) {
	return regex;
    }

    // check for normal form
    static std::regex normal_form("/.*/[i!]*", std::regex::optimize);
    if (std::regex_match(regex, normal_form)) {
	return regex;
    }

    // check for simple negative form
    if (regex[0] == '!') {
	regex[0] = '/';
	return regex + "/!";
    }

    return "/" + regex + "/";
}

std::string get_regex_flags(std::string str)
{
    if (str.size() < 3) {
	return "";
    }

    std::string flags;
    unsigned last_idx = str.size() - 1;
    while(last_idx >= 2) {
	const char last_c = str[last_idx];
	if (last_c == '/') {
	    break;
	}
	flags += last_c;
	--last_idx;
    }
    return flags;
}

std::string get_regex_str(std::string str)
{
    if (str.size() < 2) {
	return str;
    }
    if (str[0] == '/') {
	str.erase(0,1);
    }
    std::string::size_type pos = str.rfind('/');
    if (pos != std::string::npos) {
	str.erase(pos);
    }
    return str;
}

#include <iostream>
bool
is_df_with_curses_attr(const std::string& str, uint64_t& attr, int& fg, int& bg)
{
    static std::map<std::string, uint64_t> str2attr = {
	{"normal", A_NORMAL },
	{"standout", A_STANDOUT },
	{"underline", A_UNDERLINE },
	{"reverse", A_REVERSE },
	{"blink", A_BLINK },
	{"dim", A_DIM },
	{"bold", A_BOLD },
	{"italic", A_ITALIC }
    };
    static std::map<std::string, uint64_t> str2color = {
	{"black", COLOR_BLACK },
	{"red", COLOR_RED },
	{"green", COLOR_GREEN },
	{"yellow", COLOR_YELLOW },
	{"blue", COLOR_BLUE },
	{"magenta", COLOR_MAGENTA },
	{"cyan", COLOR_CYAN },
	{"white", COLOR_WHITE }
    };
#define COLORS "(black|red|green|yellow|blue|magenta|cyan|white)"
#define COL_ON_COL "(?:" COLORS " on " COLORS ")"
#define ATTRS "(normal|standout|underline|reverse|blink|dim|bold|italic)"
    static std::regex df_attr_form("\\|.*\\|(" ATTRS "(," ATTRS ")*(?:," COL_ON_COL ")?)", std::regex::optimize);
#undef COLORS
#undef ATTRS
#undef COL_ON_COL

    std::smatch m;
    if (! std::regex_match(str, m, df_attr_form)) {
	return false;
    }

    attr = 0;
    fg = bg = -1;

    for(unsigned i = 1; i < m.size(); ++i) {
	std::clog << "match " << i << ":" << m[i] << std::endl;
	auto it = str2attr.find(m[i]);
	if (it != str2attr.end()) {
	    std::clog << "or " << it->first << " " << it->second << std::endl;
	    attr |= it->second;
	    continue;
	}
	it = str2color.find(m[i]);
	if (it != str2color.end()) {
	    if (fg == -1) {
		fg = it->second;
	    } else {
		bg = it->second;
	    }
	}
    }

    // either set both fg and bg or set none.
    assert((fg==-1 && bg==-1) || (fg>=0 && bg>=0));
    return true;
}
