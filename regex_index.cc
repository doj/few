/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#include "regex_index.h"
#include "normalize_regex.h"
#include <iostream>

void convert(const std::string& flags, std::regex_constants::syntax_option_type& fl, bool& positiveMatch)
{
    bool positive_match = true;
    std::regex_constants::syntax_option_type flg = std::regex::ECMAScript | std::regex::optimize;
    for(auto f : flags) {
	switch(f) {
	case 'i': flg |= std::regex::icase; break;
	case '!': positive_match = false; break;

	default:
	    throw std::runtime_error(std::string("invalid regular expression flags character: ") + f);
	}
    }

    // set output parameters
    positiveMatch = positive_match;
    fl = flg;
}

regex_index::regex_index(std::string rgx) :
    positive_match_(true)
{
    rgx = normalize_regex(std::move(rgx));
    const std::string flags = get_regex_flags(rgx);
    rgx = get_regex_str(std::move(rgx));
    std::regex_constants::syntax_option_type fl;
    convert(flags, fl, positive_match_);
    rgx_.assign(std::move(rgx), fl);
}

void
regex_index::match(const line_t& line)
{
    //std::clog << line.num_ << ":" << line.to_string();
    const bool res = std::regex_search(line.beg_, line.end_, rgx_);
    if (( positive_match_ &&  res) ||
	(!positive_match_ && !res)) {
	lineNum_vector_.push_back(line.num_);
	//std::clog << " !match!";
    }
    //std::clog << std::endl;
}
