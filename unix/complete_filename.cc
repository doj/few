/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#define _BSD_SOURCE 1

#include "complete_filename.h"
#include <unistd.h>
#include <dirent.h>
#include <cassert>
#include <cstring>
#include <errno.h>

namespace {
    /**
     * @return the current working directory.
     */
    std::string my_getcwd()
    {
	std::string s(PATH_MAX, ' ');
	while(true) {
	    char *ret = getcwd(&s[0], s.size());
	    if (ret) {
		s.resize(strlen(ret));
		return s;
	    }
	    if (s.size() > 1000000) {
		break;
	    }
	    s.resize(s.size() * 2);
	}
	return "";
    }
}

std::set<std::string>
complete_filename(const std::string& path, std::string& err)
{
    std::set<std::string> s;

    std::string display_dirname = path,
		working_dirname = path,
		       filename = path;
    auto pos = path.rfind('/');
    if (pos == std::string::npos) {
	working_dirname = my_getcwd();
	display_dirname.clear();
    } else {
	working_dirname.erase(pos + 1);
	display_dirname.erase(pos + 1);
	filename.erase(0, pos + 1);
    }

    assert(working_dirname.size() > 0);

    auto dir = opendir(working_dirname.c_str());
    if (!dir) {
	err = "could not access directory ";
	err += working_dirname;
	err += ": ";
	err += strerror(errno);
	return s;
    }

    while(true) {
	// \todo make this function thread safe by using readdir_r()
	auto d = readdir(dir);
	if (!d) {
	    break;
	}

	// check for uninteresting names
	std::string d_name = d->d_name;
	if (d_name.empty()) {
	    continue;
	}
	if (d_name == ".") {
	    continue;
	}
	if (d_name == "..") {
	    continue;
	}

	// check for editor backup files
	pos = d_name.rfind('~');
	if (pos != std::string::npos && pos == d_name.size()-1) {
	    continue;
	}

	// check for match
	if (d_name.find(filename) == 0) {
	    std::string name = display_dirname + d_name;
	    // if the match is a directory, append a /
	    if (d->d_type == DT_DIR) {
		name += '/';
	    }
	    s.insert(name);
	}
    }
    closedir(dir);

    return s;
}
