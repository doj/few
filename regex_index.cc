/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#include "regex_index.h"
#include <regex>
#include <iostream>
regex_index::regex_index(std::shared_ptr<file_index> f_idx, const std::string& rgx, const std::string& flags)
{
    bool positive_match = true;
    std::regex_constants::syntax_option_type fl = std::regex::ECMAScript | std::regex::optimize;
    for(auto f : flags) {
	switch(f) {
	case 'i': fl |= std::regex::icase; break;
	case '!': positive_match = false; break;

	default:
	    throw std::runtime_error(std::string("invalid regex_index flags character: ") + f);
	}
    }

    std::regex r(rgx, fl);
    for(auto line : *f_idx) {
	//std::clog << line.num_ << ":" << line.to_string();
	bool res = std::regex_search(line.beg_, line.end_, r);
	if (( positive_match &&  res) ||
	    (!positive_match && !res)) {
	    lineNum_set_.insert(line.num_);
	    //std::clog << " !match!";
	}
	//std::clog << std::endl;
    }
}

lineNum_set_t
regex_index::intersect(const lineNum_set_t& s)
{
    lineNum_set_t res;
    if (! s.empty()) {
	for(auto i : lineNum_set_) {
	    if (s.count(i)) {
		res.insert(i);
	    }
	}
    }
    return res;
}
