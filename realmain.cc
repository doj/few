/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#include <sysexits.h>
#include <iostream>
#include "memorymap.h"

int realmain(int argc, const char* argv[])
{
    if (argc < 2) {
	return EX_USAGE;
    }
    if (argv == nullptr) {
	return EX_USAGE;
    }

    doj::memorymap_ptr<char> file_ptr(argv[1]);
    if (file_ptr.empty()) {
	std::cerr << "could not memory map: " << argv[1] << std::endl;
	return EX_NOINPUT;
    }

    return 0;
}
