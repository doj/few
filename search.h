/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#pragma once
#include "display_info.h"
#include "file_index.h"
#include <regex>

/**
 * search for the next occurance of the regular expression rgx in di using the lines from fi.
 * @return true if a match was found; false otherwise.
 */
bool search_next(std::wregex rgx, DisplayInfo::ptr_t di, file_index::ptr_t fi);

/**
 * search for the previous occurance of the regular expression rgx in di using the lines from fi.
 * @return true if a match was found; false otherwise.
 */
bool search_prev(std::wregex rgx, DisplayInfo::ptr_t di, file_index::ptr_t fi);
