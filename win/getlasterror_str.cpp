/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
* vi: set shiftwidth=4 tabstop=8:
* :indentSize=4:tabSize=8:
*/
#include "getlasterror_str.h"
#include "../to_wide.h"
#include <Windows.h>
#include <cassert>

std::string
GetLastError_str()
{
    const DWORD last_err = GetLastError();
    std::string s = "Windows Error: " + std::to_string(last_err);
    LPTSTR pTemp = NULL;
    unsigned retSize = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
	FORMAT_MESSAGE_FROM_SYSTEM |
	FORMAT_MESSAGE_ARGUMENT_ARRAY,
	nullptr,
	last_err,
	LANG_NEUTRAL,
	(LPTSTR)&pTemp,
	0,
	nullptr);
    if (retSize == 0 || pTemp == NULL) {
	return s;
    }
    assert(retSize > 2);
    if (pTemp[retSize - 1] == '\n') {
	pTemp[retSize - 1] = 0;
    }
    if (pTemp[retSize - 2] == '\r') {
	pTemp[retSize - 2] = 0;
    }
    s += ": ";
    s += to_utf8(pTemp);
    LocalFree((HLOCAL)pTemp);
    return s;
}
