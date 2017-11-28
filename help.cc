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
    std::cout << "usage: few [--regex '/REGEX/flags']* [--search '/REGEX/flags'] [--tabwidth 'NUM'] [--goto 'NUM'] [-v] [--color] [-h|-?|--help] ['FILE']\n"
	      << "--regex     preset Display Regular Expression or Filter Regular Expression or Attribute Display Filter Regular Expression\n"
	      << "--search    preset search regular expression\n"
	      << "--tabwidth  set the width of a tab character in spaces\n"
	      << "--goto      go to a line number\n"
	      << " -v         increase verbosity\n"
	      << "--color     enable color\n"
	      << "--help      show this text\n"
	      << "Study the man page few(1) for more details.\n"
	;
}
