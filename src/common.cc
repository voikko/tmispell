#include <string>
#include <stdio.h>
#include <stdarg.h>

#include <common.hh>

#include "glibmm/ustring.h"
#include "glibmm/unicode.h"

/**
 * Convert the given string to lowercase in place.
 */
void tolower(Glib::ustring& str) 
{
	Glib::ustring new_str;
	Glib::ustring::const_iterator i;

	for (i = str.begin();
	     i != static_cast<Glib::ustring::const_iterator>(str.end()); ++i) {
		new_str += Glib::Unicode::tolower(*i);
	}
	
	str.assign(new_str);
}

/**
 * Convert the given string to uppercase in place.
 */
void toupper(Glib::ustring& str)
{
	Glib::ustring new_str;
	Glib::ustring::const_iterator i;
	
	for (i = str.begin();
	     i != static_cast<Glib::ustring::const_iterator>(str.end()); ++i) {
		new_str += Glib::Unicode::toupper(*i);
	}
	
	str.assign(new_str);
}

/**
 * Printf something in a std::string.
 * Note that the maximum string length is 4096 chars (excluding null).
 */
std::string ssprintf(char const* fmt, ...)
{
    char buf[4096];
    va_list va;
    va_start(va, fmt);
    vsnprintf(buf, 4096, fmt, va);
    va_end(va);
    return std::string(buf);
}
