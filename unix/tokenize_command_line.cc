/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
* vi: set shiftwidth=4 tabstop=8:
* :indentSize=4:tabSize=8:
*/
#include "tokenize_command_line.h"
#include <stdlib.h>
#include <wordexp.h>
#include <iostream>
#include <cassert>
#include <cstring>

bool
tokenize_command_line(const std::string& str, int& argc, char** &argv)
{
    if (str.empty()) {
	argc = 0;
	argv = (char**) malloc(sizeof(char*));
	*argv = nullptr;
	return true;
    }

    bool ret = false;
    wordexp_t p = { 0 };
    const int r = wordexp(str.c_str(), &p, WRDE_SHOWERR | WRDE_UNDEF);
    if (r == 0) {
	argv = (char**) malloc(sizeof(char*) * (p.we_wordc + 1));
	// how stupid that we have to duplicate this entire list.
	// we should really just assign p->we_wordv to argv and skip calling wordfree() below.
	for(argc = 0; p.we_wordv[argc]; ++argc) {
	    argv[argc] = strdup(p.we_wordv[argc]);
	}
	assert(argc == static_cast<int>(p.we_wordc));
	argv[argc] = nullptr;
	ret = true;
    } else {
	switch(r) {
	case WRDE_BADCHAR: std::cerr << "Illegal occurrence of newline or one of |, &, ;, <, >, (, ), {, }." << std::endl; break;
	case WRDE_BADVAL: std::cerr << "An undefined shell variable was referenced." << std::endl; break;
	case WRDE_CMDSUB: std::cerr << "Command substitution occurred." << std::endl; break;
	case WRDE_NOSPACE: std::cerr << "Out of memory." << std::endl; break;
	case WRDE_SYNTAX: std::cerr << "Shell syntax error, such as unbalanced parentheses or unmatched quotes." << std::endl; break;
	default: std::cerr << "unknown command line parsing error." << std::endl; break;
	}
    }

    wordfree(&p);
    return ret;
}


void
tokenize_command_line_free(char* *argv)
{
    if (! argv) return;
    for(char* *a = argv; *a; ++a) {
	free(*a);
    }
    free(argv);
}
