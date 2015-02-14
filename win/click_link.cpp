/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
* vi: set shiftwidth=4 tabstop=8:
* :indentSize=4:tabSize=8:
*/
#include "../click_link.h"
#include "../to_wide.h"
#include <Windows.h>
#include <Shellapi.h>

bool click_link(const std::string& link, std::string& error_msg)
{
    std::wstring wlink = to_wide(link);
    HINSTANCE h = ShellExecute(
	nullptr, //  HWND hwnd
	L"open", // LPCTSTR lpOperation
	wlink.c_str(), // LPCTSTR lpFile
	NULL, // LPCTSTR lpParameters
	NULL, // LPCTSTR lpDirectory
	SW_SHOWNORMAL // INT nShowCmd
	);
    const int h_int = (int) h;
    if (h_int < 32) {
	switch (h_int) {
	case 0: error_msg = "The operating system is out of memory or resources."; break;
	case ERROR_FILE_NOT_FOUND: error_msg = "The specified file was not found."; break;
	case ERROR_PATH_NOT_FOUND: error_msg = "The specified path was not found."; break;
	case ERROR_BAD_FORMAT: error_msg = "The.exe file is invalid(non - Win32.exe or error in.exe image)."; break;
	case SE_ERR_ACCESSDENIED: error_msg = "The operating system denied access to the specified file."; break;
	case SE_ERR_ASSOCINCOMPLETE: error_msg = "The file name association is incomplete or invalid."; break;
	case SE_ERR_DDEBUSY: error_msg = "The DDE transaction could not be completed because other DDE transactions were being processed."; break;
	case SE_ERR_DDEFAIL: error_msg = "The DDE transaction failed."; break;
	case SE_ERR_DDETIMEOUT: error_msg = "The DDE transaction could not be completed because the request timed out."; break;
	case SE_ERR_DLLNOTFOUND: error_msg = "The specified DLL was not found."; break;
	//case SE_ERR_FNF: error_msg = "The specified file was not found."; break;
	case SE_ERR_NOASSOC: error_msg = "There is no application associated with the given file name extension.This error will also be returned if you attempt to print a file that is not printable."; break;
	case SE_ERR_OOM: error_msg = "There was not enough memory to complete the operation."; break;
	//case SE_ERR_PNF: error_msg = "The specified path was not found."; break;
	case SE_ERR_SHARE: error_msg = "A sharing violation occurred."; break;
	default: error_msg = "unknown ShellExecute() error: " + std::to_string(h_int); break;
	}
	return false;
    }
    return true;
}

bool click_email(const std::string& email, const std::string& subject, std::string& error_msg)
{
    std::wstring cmd = L"mailto:";
    cmd += to_wide(email);

    HINSTANCE h = ShellExecute(
	nullptr, //  HWND hwnd
	L"open", // LPCTSTR lpOperation
	cmd.c_str(), // LPCTSTR lpFile
	NULL, // LPCTSTR lpParameters
	NULL, // LPCTSTR lpDirectory
	SW_SHOWNORMAL // INT nShowCmd
	);
    const int h_int = (int)h;
    if (h_int < 32) {
	switch (h_int) {
	case 0: error_msg = "The operating system is out of memory or resources."; break;
	case ERROR_FILE_NOT_FOUND: error_msg = "The specified file was not found."; break;
	case ERROR_PATH_NOT_FOUND: error_msg = "The specified path was not found."; break;
	case ERROR_BAD_FORMAT: error_msg = "The.exe file is invalid(non - Win32.exe or error in.exe image)."; break;
	case SE_ERR_ACCESSDENIED: error_msg = "The operating system denied access to the specified file."; break;
	case SE_ERR_ASSOCINCOMPLETE: error_msg = "The file name association is incomplete or invalid."; break;
	case SE_ERR_DDEBUSY: error_msg = "The DDE transaction could not be completed because other DDE transactions were being processed."; break;
	case SE_ERR_DDEFAIL: error_msg = "The DDE transaction failed."; break;
	case SE_ERR_DDETIMEOUT: error_msg = "The DDE transaction could not be completed because the request timed out."; break;
	case SE_ERR_DLLNOTFOUND: error_msg = "The specified DLL was not found."; break;
	    //case SE_ERR_FNF: error_msg = "The specified file was not found."; break;
	case SE_ERR_NOASSOC: error_msg = "There is no application associated with the given file name extension.This error will also be returned if you attempt to print a file that is not printable."; break;
	case SE_ERR_OOM: error_msg = "There was not enough memory to complete the operation."; break;
	    //case SE_ERR_PNF: error_msg = "The specified path was not found."; break;
	case SE_ERR_SHARE: error_msg = "A sharing violation occurred."; break;
	default: error_msg = "unknown ShellExecute() error: " + std::to_string(h_int); break;
	}
	return false;
    }
    return true;
}
