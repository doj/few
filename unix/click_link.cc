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
    if (link.empty()) {
	error_msg = "click_link(): link is empty";
	return false;
    }
    if (link.find('\'') != std::string::npos) {
	error_msg = "link contains '";
	return false;
    }

#ifdef __APPLE__
    std::string browser = "open";
#else
    std::string browser = "firefox";
#endif
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

bool click_email(const std::string& email, const std::string& subject, std::string& error_msg)
{
    if (email.empty()) {
	error_msg = "click_email(): email is empty";
	return false;
    }
    if (email.find('\'') != std::string::npos) {
	error_msg = "email contains '";
	return false;
    }
    if (subject.find('\'') != std::string::npos) {
	error_msg = "subject contains '";
	return false;
    }

    std::string cmd;
#ifdef __APPLE__
    cmd = "open 'mailto:";
    cmd += email;
    cmd += '\'';
#else
    std::string mailer = "thunderbird";
    const char *cc = getenv("MAILER");
    if (cc) {
	mailer = cc;
	if (mailer.empty()) {
	    error_msg = "MAILER environment variable is empty";
	    return false;
	}
    }
    assert(! mailer.empty());
    cmd = mailer + " '" + email + "'";
    if (mailer == "thunderbird") {
	// http://kb.mozillazine.org/Command_line_arguments_%28Thunderbird%29
	cmd = mailer + " -compose \"to='" + email + "',subject='" + subject + "'\"";
    } else if (mailer == "evolution") {
	cmd = mailer + " 'mailto:" + email + "?subject=" + subject + "'";
    }
#endif

    if (! run_command_background(cmd, error_msg)) {
	return false;
    }
    return true;
}
