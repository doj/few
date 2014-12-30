/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
* vi: set shiftwidth=4 tabstop=8:
* :indentSize=4:tabSize=8:
*/
#pragma once
#include <string>

/**
 * parse a string like the shell would parse a command line.
 *
 * The empty string is a valid input, it results in a return value of
 * true and argc is set to 0. argv will contain a list with a single
 * element, the nullptr. The caller has to free argv with the
 * tokenize_command_line_free() function.
 *
 * If the parsing of str fails, the return value is false and argc and
 * argv are not modified. The caller does not have to free anything if
 * the function returns false. If parsing fails the function may write
 * an error message to STDERR.
 *
 * @param[in] str input string.
 * @param[out] argc upon success, number of entries in argv.
 * @param[out] argv upon success, list of C-string pointers. The caller has to free this list by calling tokenize_command_line_free(argv). The last pointer in the list is a nullptr.
 * @return true upon sucess; false upon failure.
 */
bool tokenize_command_line(const std::string& str, int& argc, char** &argv);

/// free a string list created by the tokenize_command_line() function.
void tokenize_command_line_free(char **argv);
