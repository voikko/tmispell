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

#include <stdarg.h>
#include <stdio.h>

#include "tmerror.hh"

Error::Error(const char* fmt, ...)
	: std::runtime_error("")
{
	char buf[4096];
	va_list va;
	va_start(va, fmt);
	vsnprintf(buf, 4096, fmt, va);
	va_end(va);
	throw Error(std::string(buf));
}

Error::Error(std::string const& msg)
	: std::runtime_error(msg)
{
}
