/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#include <iostream>
/**
 * print command line help text to stdout.
 * \todo keep this in sync with README.md
 */
void help()
{
    std::cout << "usage: fewer [--regex 'REGEX']* [--tabwidth 'NUM'] 'FILE'" << std::endl
	      << "--regex      preset regular expressions to filter the file." << std::endl
	      << "--tabwidth   set the width of a tab character in spaces." << std::endl
	;
}
