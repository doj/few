/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
* vi: set shiftwidth=4 tabstop=8:
* :indentSize=4:tabSize=8:
*/

#include "../console.h"
#include "../types.h"
#include "../errno.h"
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>

bool open_tty_as_stdin()
{
    // open the CON device and make it STDIN
    int fd = -1;
    errno_t e = _sopen_s(&fd, "con", _O_RDONLY, _SH_DENYNO, _S_IREAD | _S_IWRITE);
    if (e != 0 || fd < 0) {
	std::cerr << "could not open con: " << errno_str() << std::endl;
	return false;
    }
    if (fd != 0) {
	if (_dup2(fd, 0) < 0) {
	    std::cerr << "could not dup con to STDIN: " << errno_str() << std::endl;
	    return false;
	}
	_close(fd);
    }

    return true;
}
