/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
* vi: set shiftwidth=4 tabstop=8:
* :indentSize=4:tabSize=8:
*/
#pragma once

/**
 * merge the right argv list into the left argv list.

 * The left argv list starts with the program name and may be followed
 * by command line arguments and the last element is a nullptr. The
 * right argv list only contains command line arguments and the last
 * element is a nullptr. The right argv is inserted between the
 * program name and the left argv command line arguments.
 *
 * This function does not alter the strings in argv_left or
 * argv_right. It may create a new list of strings which is returned
 * in the argv_left parameter. The caller is responsible to free any
 * strings or lists. If the function returns false, No parameters are
 * changed and no memory is allocated.
 *
 * @param[in,out] argc_left number of elements in argv_left, not including the last nullptr.
 * @param[in,out] argv_left list of strings, the last element is nullptr. The list contains argc_left+1 elements.
 * @param[in] argc_right number of elements in argv_right, not including the last nullptr.
 * @param[in] argv_right list of strings, the last element is nullptr. The list contains argc_right+1 elements.
 *
 * @return true upon success; false upon failure.
 */
bool merge_command_line_lists(int &argc_left, const char** &argv_left, const int argc_right, const char** argv_right);
