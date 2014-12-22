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
