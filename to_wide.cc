/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#include "to_wide.h"
#include <cassert>
#include <cstring>

std::wstring to_wide(const std::string& s)
{
    std::wstring out;
    if (s.empty()) {
	return out;
    }

    auto src = s.c_str();
    while(true) {
	std::wstring wline(s.size(), L'\0');
	mbstate_t ps; memset(&ps, 0, sizeof(ps));
	auto w_line_size = mbsrtowcs(const_cast<wchar_t*>(wline.c_str()), &src, wline.size(), &ps);
	if (w_line_size == (size_t) -1) {
	    assert(errno == EILSEQ);

	    // find how many wide characters seem valid
	    int i;
	    for(i = wline.size()-1; i >= 0 && wline[i] == 0; --i) { }
	    if (i >= 0) {
		wline.erase(i + 1);
		out += wline;
	    }

	    out += L'\uFFFD';
	    ++src;
	} else {
	    wline.resize(w_line_size);
	    out += wline;
	    break;
	}
    }
    return out;
}

std::string to_utf8(const std::wstring& s)
{
    const size_t len = wcstombs(nullptr, s.c_str(), 0);
    if (len == (size_t)-1) {
	return "";
    }
    std::string utf8(len, ' ');
    const size_t l = wcstombs(const_cast<char*>(utf8.data()), s.c_str(), utf8.size()+1);
    assert(l == len);
    return utf8;
}
