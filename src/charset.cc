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
 * @file charset.cc
 * @author Pauli Virtanen
 *
 * Converting strings from charset to another.
 */
#include <string>

#include "glibmm/ustring.h"
#include "glibmm/unicode.h"
#include "glibmm/convert.h"

#include <errno.h>
#include <glib/gerror.h>

#include "i18n.hh"
#include "common.hh"
#include "charset.hh"

/**
 * Private implementation for the character set converter.
 */
class CharsetConverterPimpl
{
public:
	/// Create a private implementation with the given charset
	CharsetConverterPimpl(char const* cset);
	/// Destroy a private implementation
	~CharsetConverterPimpl();

	/// Convert from external encoding to internal UTF-8
	Glib::ustring CharsetConverterPimpl::from(std::string str);
	/// Convert internal UTF-8 to external encoding
	std::string CharsetConverterPimpl::to(Glib::ustring str);

private:
	/// The name of the character set
	std::string cset_;
	/// The iconv handle to convert internal UTF-8 to external encoding
	Glib::IConv* to_;
	/// The iconv handle to convert external encoding to internal UTF-8
	Glib::IConv* from_;
};

/**
 * Create a character set converter private implementation.
 * @param cset	The character set name, as recognized by iconv.
 */
CharsetConverterPimpl::CharsetConverterPimpl(char const* cset)
	: cset_(cset)
{
	try {
		to_ = new Glib::IConv(cset, "UTF-8");
		from_ = new Glib::IConv("UTF-8", cset);
	} catch (GError* err) {
		throw Error(_("Error initializing character "
			      "set conversion: %s"), err->message);
	}
}

/** Destroy the character set converter. */
CharsetConverterPimpl::~CharsetConverterPimpl()
{
	delete to_;
	delete from_;
	to_ = 0;
	from_ = 0;
}

/**
 * Convert a string from external encoding to internal UTF-8.
 * @param str	The string in external encoding to convert
 */
Glib::ustring CharsetConverterPimpl::from(std::string str)
{
	try {
		std::string ostr = from_->convert(str);
		return Glib::ustring(ostr.c_str());
	} catch (Glib::ConvertError err) {
		throw ConvertError(cset_, "UTF-8", str,
				   err.what().c_str());
	}
}

/**
 * Convert a string from internal UTF-8 to external encoding.
 * @param str	The UTF-8 string to convert
 */
std::string CharsetConverterPimpl::to(Glib::ustring str)
{
	try {
		return to_->convert(std::string(str.data(), str.bytes()));
	} catch (Glib::ConvertError err) {
		throw ConvertError("UTF-8", cset_, str,
				   err.what().c_str());
	}
}

/**/

/**
 * Create a new charset converter but without really allocating a converter.
 */
CharsetConverter::CharsetConverter()
	: pimpl_(0)
{
}

/**
 * Create a new character set converter, using given external encoding.
 * @param cset	The character set name, as recognized by iconv.
 */
CharsetConverter::CharsetConverter(char const* cset)
{
	pimpl_ = new CharsetConverterPimpl(cset);
}

/** Destroy this character set converter */
CharsetConverter::~CharsetConverter()
{
	delete pimpl_;
	pimpl_ = 0;
}

/**
 * Convert a string from external encoding to internal UTF-8.
 * @param str	The string in external encoding to convert
 */
Glib::ustring CharsetConverter::from(std::string str)
{
	return pimpl_->from(str);
}

/**
 * Convert a string from internal UTF-8 to external encoding.
 * @param str	The UTF-8 string to convert
 */
std::string CharsetConverter::to(Glib::ustring wstr)
{
	return pimpl_->to(wstr);
}

/** The character set converter for the default locale */
CharsetConverter* CharsetConverter::locale_ = 0;

/**
 * Get the character set converter for the default locale, creating it if
 * necessary.
 */
CharsetConverter& CharsetConverter::locale()
{
	if (locale_ == 0) {
		std::string cset;
		Glib::get_charset(cset);
		locale_ = new CharsetConverter(cset.c_str());
	}
	return *locale_;
}

/**
 * Initialize a proper error message.
 */
ConvertError::ConvertError(std::string const& from,
			   std::string const& to,
			   std::string const& str,
			   std::string const& reason)
	: Error("")
{
	msg_ = ssprintf(
		_("Conversion of '%s' to character set '%s' failed: %s"),
		str.c_str(), from.c_str(), to.c_str());
}

#if TEST

#include <iostream>

int main(int argc, char** argv)
{
	locale_init();
	try {
		std::string s;
		std::cin >> s;
		std::string cset;
		Glib::get_charset(cset);
		std::cout << cset << std::endl;
		std::cout << CharsetConverter::locale().to(s).c_str()
			  << std::endl;
	} catch (Error const& err) {
		std::cerr << err.what() << std::endl;
	}
}


#endif
