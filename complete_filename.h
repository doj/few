/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#pragma once
#include <string>
#include <set>

/**
 * lookup file or directory name completions for path.
 * path can be a partial pathname.
 * Unix style editor backup files (ending with a tilde character) are not completed.
 * @param[in,out] path (partial) file or directory name. The string may be changes for a longest prefix match if matches are found.
 * @param[out] err an error message if a )file system) error happened.
 * @return file or directory name completions matching path;
 *         empty array if path does not match anything on the file system.
 */
std::set<std::string> complete_filename(std::string& path, std::string& err);
