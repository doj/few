/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
* vi: set shiftwidth=4 tabstop=8:
* :indentSize=4:tabSize=8:
*/
#include "click_link.h"
#include "command.h"
#include <cassert>
#include <cstdlib>

bool
click_link(const std::string& link, std::string& error_msg)
{
	std::string browser = "firefox";
	const char *cc = getenv("BROWSER");
	if (cc) {
	    browser = cc;
	    if (browser.empty()) {
		error_msg = "BROWSER environment variable is empty";
		return false;
	    }
	}
	assert(! browser.empty());
	if (! run_command_background(browser + " '" + link + "'", error_msg)) {
	    return false;
	}
	return true;
}
