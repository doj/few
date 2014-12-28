/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
* vi: set shiftwidth=4 tabstop=8:
* :indentSize=4:tabSize=8:
*/
#include "../to_wide.h"
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <errno.h>

std::wstring to_wide(const std::string& s)
{
    std::wstring out;
    if (s.empty()) {
	return out;
    }

    auto src = s.c_str();
    while (true) {
	std::wstring wline(s.size(), L'\0');
	mbstate_t ps; memset(&ps, 0, sizeof(ps));
	size_t w_line_size = 0;
	errno_t e = mbsrtowcs_s(&w_line_size, const_cast<wchar_t*>(wline.c_str()), wline.size() + 1u, &src, wline.size(), &ps);
	assert(e == 0);
	assert(w_line_size > 0);
	--w_line_size; // decrement the count of the trailing 0 character
	if (w_line_size == (size_t)-1) {
	    assert(errno == EILSEQ);

	    // find how many wide characters seem valid
	    int i;
	    for (i = wline.size() - 1; i >= 0 && wline[i] == 0; --i) {}
	    if (i >= 0) {
		wline.erase(i + 1);
		out += wline;
	    }

	    out += L'\uFFFD';
	    ++src;
	}
	else {
	    wline.resize(w_line_size);
	    out += wline;
	    break;
	}
    }
    return out;
}

std::string to_utf8(const std::wstring& s)
{
    // first get size of target UTF-8 string
    size_t len = 0, l = 0;
    errno_t e = wcstombs_s(&len, nullptr, 0, s.c_str(), _TRUNCATE);
    if (e != 0) {
	return "";
    }
    assert(len > 0);
    --len; // subtract terminating NUL characters
    if (len == (size_t)-1) {
	return "";
    }

    // create UTF-8 string
    std::string utf8(len, ' ');
    e = wcstombs_s(&l, const_cast<char*>(utf8.data()), utf8.size() + 1u, s.c_str(), s.size());
    if (e != 0) {
	return "";
    }
    assert(l > 0);
    --l; // subtract trailing NUL character
    assert(l == len);
    return utf8;
}
