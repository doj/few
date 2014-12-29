/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
* vi: set shiftwidth=4 tabstop=8:
* :indentSize=4:tabSize=8:
*/

#include "errno_str.h"
#include <errno.h>
#include <cstring>

std::string errno_str()
{
    return strerror(errno);
}
