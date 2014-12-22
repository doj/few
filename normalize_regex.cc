/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#include "normalize_regex.h"
#include <regex>

std::string normalize_regex(std::string regex)
{
    if (regex.empty()) {
	return regex;
    }

    if (regex == "/") {
	return "///";
    }

    if (regex == "!") {
	return "/!/";
    }

    std::regex regular_form("/.*/[i!]*");
    if (std::regex_match(regex, regular_form)) {
	return regex;
    } else if (regex[0] == '!') {
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
