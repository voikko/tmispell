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
