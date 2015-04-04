/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#include "word_set.h"
#include <cctype>
#include <cassert>

namespace {
    std::set<std::string> word_set;

    bool is_word_character(int c)
    {
	return isalnum(c) || c=='_' || c=='-';
    }
}

void
add_to_word_set(const std::string& line)
{
    std::string::const_iterator it = line.begin();
    std::string::const_iterator beg = it, end = it;
    while(it != line.end()) {

	// search for start of word
	while(it != line.end() && !is_word_character(*it)) {
	    ++it;
	}
	beg = it;

	// search for end of word
	while(it != line.end() && is_word_character(*it)) {
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
complete_word_set(std::string& word, std::string& err)
{
    std::set<std::string> s;
    if (word.empty()) {
	s = word_set;
    } else {
	for(const auto& w : word_set) {
	    if (w.find(word) == 0) {
		s.insert(w);
	    }
	}
    }

    complete_longest_prefix(word, s);
    return s;
}

void
complete_longest_prefix(std::string& str, const std::set<std::string>& s)
{
    // if the set is empty, there's nothing to do
    if (s.empty()) {
	return;
    }
    // if the set has only one element?
    if (s.size() == 1) {
	auto it = s.begin();
	// if str is empty, set str to the element
	if (str.empty()) {
	    str = *it;
	    // if str is prefixing the element, set str to the element
	} else if (it->find(str) == 0) {
	    str = *it;
	}
	// else: str does not prefix the element, don't modify str.
	return;
    }

    assert(s.size() > 1);
    std::string prefix;
    for(const auto& element : s) {
	if (element.find(str) != 0) {
	    continue;
	}
	if (prefix.empty()) {
	    prefix = element;
	    continue;
	}
	unsigned i;
	for(i = 0; i < element.size() && i < prefix.size() && element[i] == prefix[i]; ++i) {}
	prefix.erase(i);
    }
    if (! prefix.empty()) {
	str = prefix;
    }
}
