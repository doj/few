#include "../complete_filename.h"
#include "../to_wide.h"
#include "getlasterror_str.h"
#include "../word_set.h"
#include <direct.h>
#include <cassert>
#include <windows.h>

std::set<std::string>
complete_filename(std::string& path, std::string& err)
{
    std::set<std::string> s;

    std::wstring display_dirname = to_wide(path);
    std::wstring working_dirname = display_dirname, filename = display_dirname;
    auto pos = path.rfind('\\');
    if (pos == std::string::npos) {
	wchar_t *p = _wgetcwd(nullptr, 0);
	working_dirname = p;
	working_dirname += L"\\*";
	free(p);
	display_dirname.clear();
    } else {
	working_dirname.erase(pos + 1);
	working_dirname += L"*";
	display_dirname.erase(pos + 1);
	filename.erase(0, pos + 1);
    }

    assert(working_dirname.size() > 0);

    WIN32_FIND_DATA ffd;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    hFind = FindFirstFile(working_dirname.c_str(), &ffd);
    if (hFind == INVALID_HANDLE_VALUE)
    {
	err = "could not access directory ";
	err += to_utf8(working_dirname);
	err += ": ";
	err += GetLastError_str();
	return s;
    }

    do {
	// check for uninteresting names
	std::wstring d_name = ffd.cFileName;
	if (d_name.empty()) {
	    goto next_file;
	}
	if (d_name == L".") {
	    goto next_file;
	}
	if (d_name == L"..") {
	    goto next_file;
	}

	// check for editor backup files
	pos = d_name.rfind(L'~');
	if (pos != std::string::npos && pos == d_name.size() - 1) {
	    goto next_file;
	}

	// check for match
	if (d_name.find(filename) == 0) {
	    std::string name = to_utf8(display_dirname + d_name);
	    // if the match is a directory, append a /
	    if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		name += '\\';
	    }
	    s.insert(name);
	}
    next_file:;
    } while (FindNextFile(hFind, &ffd));

    DWORD dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES)
    {
	err = "error while iterating directory ";
	err += to_utf8(working_dirname);
	err += ": ";
	err += GetLastError_str();
    }

    FindClose(hFind);
    complete_longest_prefix(path, s);
    return s;
}
