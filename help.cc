/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#include <iostream>
void help()
{
    std::cout << "usage: fewer [--regex <regular expression>]* [--tabwidth <num>] <filename>" << std::endl
	      << "--regex      preset regular expressions to filter the fil" << std::endl
	      << "--tabwidth   configure the width of a tab character in spaces" << std::endl
	;
}
