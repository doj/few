/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#include "console.h"
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <errno.h>

bool open_tty_as_stdin()
{
    // now directly open the TTY device, since STDIN is no longer available for key input
    int fd = open("/dev/tty", O_RDONLY);
    if (fd < 0) {
	std::cerr << "could not open /dev/tty: " << strerror(errno) << std::endl;
	return false;
    }
    if (fd != 0) {
	if (dup2(fd, 0) < 0) {
	    std::cerr << "could not dup /dev/tty to STDIN: " << strerror(errno) << std::endl;
	    return false;
	}
	close(fd);
    }

    return true;
}
