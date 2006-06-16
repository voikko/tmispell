/* Copyright (C) Pauli Virtanen
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *********************************************************************************/

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
