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
    if (is_attr_df(regex, attr, fg, bg)) {
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

std::string get_regex_str(const std::string& str)
{
    if (str.size() < 2) {
	return str;
    }

    std::string o;
    for(unsigned i = ((str[0] == '/') ? 1 : 0);
	i < str.size();
	++i) {
	const char c = str[i];
	if (c == '\\') {
	    if (i == str.size()-1) {
		o += c;
		break;
	    }
	    const char n = str[++i];
	    if (n == '/' || n == '\\') {
		o += n;
		continue;
	    } else {
		return "";
	    }
	}
	if (c == '/') {
	    break;
	}
	o += c;
    }

    return o;
}

#include <iostream>
bool
is_attr_df(const std::string& str, uint64_t& attr, int& fg, int& bg)
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
#define COLORS "(?:black|red|green|yellow|blue|magenta|cyan|white)"
#define COL_ON_COL "(" COLORS " on " COLORS ")"
#define ATTRS "(?:normal|standout|underline|reverse|blink|dim|bold|italic)"
    static std::regex df_attr_form("\\|.*\\|(" ATTRS "(?:," ATTRS ")*)(?:," COL_ON_COL ")?", std::regex::optimize);

    std::smatch m;
    if (! std::regex_match(str, m, df_attr_form)) {
	return false;
    }

    attr = 0;
    fg = bg = -1;

#if 0
    for(unsigned i = 1; i < m.size(); ++i) {
	std::clog << "match " << i << ":" << m[i] << std::endl;
    }
#endif

    // match attributes
    if (m.size() > 1) {
	static std::regex rgx(",?(" ATTRS ")", std::regex::optimize);
	const std::string s = m[1];
	for(std::sregex_iterator it(s.begin(), s.end(), rgx), it_end; it != it_end; ++it) {
	    auto i = str2attr.find((*it)[1]);
	    if (i != str2attr.end()) {
		//std::clog << "or " << i->first << " " << i->second << std::endl;
		attr |= i->second;
	    }
	}
    }

    // match colors
    if (m.size() > 2) {
	static std::regex rgx("(" COLORS ") on (" COLORS ")", std::regex::optimize);
	std::smatch c;
	const std::string s = m[2];
	if (std::regex_match(s, c, rgx)) {
	    auto it = str2color.find(c[1]);
	    assert(it != str2color.end());
	    fg = static_cast<int>(it->second);

	    it = str2color.find(c[2]);
	    assert(it != str2color.end());
	    bg = static_cast<int>(it->second);
	}
    }

    // either set both fg and bg or set none.
    assert((fg==-1 && bg==-1) || (fg>=0 && bg>=0));
    return true;

#undef COLORS
#undef ATTRS
#undef COL_ON_COL
}

bool is_filter_regex(std::string str)
{
    str = normalize_regex(str);
    if (str.size() < 3) {
	return false;
    }

    unsigned slash_cnt = 0;
    for(unsigned i = 0; i < str.size(); ++i) {
	const char c = str[i];
	if (c == '/') { ++slash_cnt; continue; }
	// check for valid flags after 2nd forward slash
	if (slash_cnt >= 2) {
	    if (c == 'i' || c == '!') { continue; }
	    return false;
	}
	// check for escapes
	if (c == '\\') {
	    if (i == str.size() - 1) { return false; }
	    assert(i < str.size()-1);
	    const char n = str[++i];
	    if (n == '/' || n == '\\') { continue; }
	    return false;
	}
    }

    if (slash_cnt != 2) {
	return false;
    }
    return true;

#if 0
    static std::regex r("^/.+/[i!]*");
    if (std::regex_match(str, r)) {
	return true;
    }
    return false;
#endif
}

bool parse_replace_df(const std::string& expr, std::string& rgx, std::string& rpl, std::string& err_msg)
{
    // the expression needs to contain at least:
    // 3 slashes
    // one character for rgx
    if (expr.size() < 3+1) {
	err_msg = "regex has not enough characters";
	return false;
    }

    // expr needs to start with a slash
    if (expr[0] != '/') {
	err_msg = "does not start with /";
	return false;
    }

    // expr needs to end with a slash
    if (expr[expr.size() - 1] != '/') {
	err_msg = "does not end with /";
	return false;
    }

    // the number of slashes found
    unsigned slash_cnt = 0;

    enum mode_t { RGX, RPL } mode = RGX;
    std::string rgx_, rpl_;
    for(unsigned i = 0; i < expr.size(); ++i) {
	char c = expr[i];
	// do we have a slash?
	if (c == '/') {
	    ++slash_cnt;
	    // the second slash sets the RPL mode
	    if (slash_cnt == 2) {
		assert(mode == RGX);
		mode = RPL;
	    }
	    continue;
	}
	// do we have a backslash?
	if (c == '\\') {
	    assert(i < expr.size()-1);
	    // if next character is a forward slash or backslash, process that
	    const char n = expr[++i];
	    if (n == '/' || n == '\\') {
		c = n;
	    } else {
		// no other escapes supported
		err_msg = "invalid escaped character: ";
		err_msg += n;
		return false;
	    }
	}
	// add current character to correct output string
	if (mode == RGX) {
	    rgx_ += c;
	} else if (mode == RPL) {
	    rpl_ += c;
	} else {
	    assert(false && "unknown mode");
	}
    }

    if (slash_cnt != 3) {
	err_msg = "did not find 3 forward slashes";
	return false;
    }

    rgx = rgx_;
    rpl = rpl_;
    return true;
}
