/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#pragma once

#include <string>
#include <set>

/**
 * parse line for words and add to the word set.
 * the words are separated by white space or punctuation characters.
 */
void add_to_word_set(const std::string& line);

/// clear the word set
void clear_word_set();

/**
 * look up auto completitions from the word set.

 * @param[in,out] word word that should be completed. The input value
 * is used to look up matches in the word set. The word may be
 * modified to the longest prefix of matches found.
 *
 * @param[out] err error string.
 * @return set of matched words.
 */
std::set<std::string> complete_word_set(std::string& word, std::string& err);

/**
 * look up the longest prefix match of str in s and set str to that prefix.
 * @param[in,out] str a string.
 * @param[in] s a set of strings which are searched for the longest prefix.
 */
void complete_longest_prefix(std::string& str, const std::set<std::string>& s);
