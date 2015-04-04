/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#pragma once

#include <string>
#include <set>

/**
 * parse line for words and add to the word set.
 * the words are separated by white space.
 */
void add_to_word_set(const std::string& line);

/// clear the word set
void clear_word_set();

/// look up auto completitions from the word set.
std::set<std::string> complete_word_set(const std::string& word, std::string& err);
