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

/**
 * @file charset.hh
 * @author Pauli Virtanen
 *
 * Converting strings from charset to another.
 */
#ifndef CHARSET_HH_
#define CHARSET_HH_

#include <string>

#include "tmerror.hh"

#include "glibmm/ustring.h"

class CharsetConverterPimpl;

/**
 * Character set converter. Converts from an external encoding to
 * the internally used UTF-8 and vice versa.
 */
class CharsetConverter
{
public:
	/// Create a character set converter, with given external charset
	CharsetConverter(char const* cset);
	/// Destroy the converter
	~CharsetConverter();

	/// Convert from external encoding to internal UTF-8
	Glib::ustring from(std::string str);
	/// Convert from internal UTF-8 to external encoding
	std::string to(Glib::ustring wstr);

	/// Get the converter corresponding to the default locale
	static CharsetConverter& locale();
	
private:
	/// Private creator
	CharsetConverter();
	
	/// The converter corresponding to the default locale
	static CharsetConverter* locale_;

	/// Private implementation for the converter
	CharsetConverterPimpl* pimpl_;
};

/**
 * An error describing problems in conversion
 */
class ConvertError : protected Error
{
public:
	ConvertError(std::string const& from, std::string const& to,
		     std::string const& str,
		     std::string const& reason);
};


#endif
