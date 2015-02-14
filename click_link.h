/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
* vi: set shiftwidth=4 tabstop=8:
* :indentSize=4:tabSize=8:
*/
#pragma once
#include <string>

/**
 * open a URL in a web browser.
 * @param[in] link a URL that is launched in a web browser.
 * @param[out] error_msg if the function returns false, this string will contain an error message.
 * @return true if a web browser was launched; false upon error.
 */
bool click_link(const std::string& link, std::string& error_msg);

/**
 * create an email.
 * @param[in] email an email address.
 * @param[in] subject the subject of the new email, can be empty.
 * @param[out] error_msg if the function returns false, this string will contain an error message.
 * @return true if an email window could be opened; false upon error.
 */
bool click_email(const std::string& email, const std::string& subject, std::string& error_msg);
