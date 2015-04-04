/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#include "word_set.h"
#include <cctype>

namespace {
    std::set<std::string> word_set;
}

void
add_to_word_set(const std::string& line)
{
    std::string::const_iterator it = line.begin();
    std::string::const_iterator beg = it, end = it;
    while(it != line.end()) {

	// search for start of word
	while(it != line.end() && isspace(*it)) {
	    ++it;
	}
	beg = it;

	// search for end of word
	while(it != line.end() && !isspace(*it)) {
	    ++it;
	}
	end = it;

	if (beg != end) {
	    word_set.insert(std::string(beg,end));
	}
    }
}

void
clear_word_set()
{
    word_set.clear();
}

std::set<std::string>
complete_word_set(const std::string& word, std::string& err)
{
    if (word.empty()) {
	return word_set;
    }

    std::set<std::string> s;
    for(const auto& w : word_set) {
	if (w.find(word) == 0) {
	    s.insert(w);
	}
    }
    return s;
}
