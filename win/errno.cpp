/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
* vi: set shiftwidth=4 tabstop=8:
* :indentSize=4:tabSize=8:
*/

#include "../errno.h"
#include <errno.h>
#include <cstring>

std::string errno_str()
{
    char buffer[1024];
    errno_t e = strerror_s(buffer, sizeof(buffer), errno);
    if (e == 0) {
	return buffer;
    }
    return "errno:" + std::to_string(errno);
}
