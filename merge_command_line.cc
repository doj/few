/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
* vi: set shiftwidth=4 tabstop=8:
* :indentSize=4:tabSize=8:
*/
#include "merge_command_line.h"
#include <cstdlib>
#include <cassert>

bool
merge_command_line_lists(int &argc_left, const char** &argv_left, const int argc_right, const char** argv_right)
{
    if (argc_left < 1) {
	return false;
    }
    if (argv_left == nullptr) {
	return false;
    }
    if (argc_right < 0) {
	return false;
    }
    if (argc_right == 0) {
	return true;
    }
    if (argv_right == nullptr) {
	return false;
    }

    assert(argc_left >= 1);
    assert(argc_right >= 1);

    const char **argv = (const char**) malloc(sizeof(char*) * (argc_left + argc_right + 1));
    if (! argv) {
	return false;
    }

    int cnt = 0;
    // copy program name from left list
    argv[cnt++] = argv_left[0];
    // copy entire right list
    int i;
    for(i = 0; i < argc_right; ++i) {
	argv[cnt++] = argv_right[i];
    }
    assert(argv_right[i] == nullptr);
    // copy remaining left list
    for(i = 1; i < argc_left; ++i) {
	argv[cnt++] = argv_left[i];
    }
    assert(argv_left[i] == nullptr);
    // add trailing nullptr
    argv[cnt] = nullptr;
    assert(cnt == argc_left + argc_right);

    argc_left = cnt;
    argv_left = argv;
    return true;
}
