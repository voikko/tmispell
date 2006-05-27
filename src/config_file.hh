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
 * @file config_file.hh
 * @author Pauli Virtanen
 *
 * Extracting information from the configuration file.
 */
#ifndef CONFIG_FILE_HH_
#define CONFIG_FILE_HH_

#include "glibmm/ustring.h"
#include <map>
#include <string>
#include <vector>

#include "tmerror.hh"

/**
 * A spellchecker module configuration.
 */
class SpellcheckerEntry
{
public:
	SpellcheckerEntry(std::string library,
			  std::string dictionary,
			  std::string encoding,
			  std::string lc_ctype,
			  std::string word_chars,
			  std::string boundary_chars);

	/// Return the library file name
	std::string const& get_library() const
		{ return library_; }

	/// Return the dictionary file name
	std::string const& get_dictionary() const
		{ return dictionary_; }

	/// Return the input encoding
	std::string const& get_encoding() const
		{ return encoding_; }

	/// Return the LC_CTYPE
	std::string const& get_lc_ctype() const
		{ return lc_ctype_; }

	/// Return the extra word characters
	std::vector<gunichar> const& get_word_chars() const
		{ return word_chars_; }

	/// Return the word boundary characters
	std::vector<gunichar> const& get_boundary_chars() const
		{ return boundary_chars_; }

private:
	std::string library_; ///< Library file name
	std::string dictionary_; ///< Dictionary file name
	std::string encoding_; ///< Input encoding
	std::string lc_ctype_; ///< LC_CTYPE
	
	std::vector<gunichar> word_chars_; ///< Extra word characters
	std::vector<gunichar> boundary_chars_; ///< Word boundary characters
};

/// Case insensitive comparison operator
struct NocaseCmp
{
	bool operator()(std::string const& s1, std::string const& s2) const;
};

/**
 * The information stored in the configuration file.
 * That is, map from dictionary identifiers to library files, dictionaries and
 * extra characters considered parts of words.
 */
class ConfigFile
{
public:
	/// Read the given configuration file
	ConfigFile(std::string const& file_name);

	/// Return the option with the given name.
	std::string const& get_option(std::string const& option_name) const;
	
	/// Return the spellchecker entry corresponding to the given id
	SpellcheckerEntry const& get(std::string const& id) const;

	/// Does the configuration file have a given spell checker entry?
	bool has(std::string const& id) const;

public:
	/// Mapping from spell checker name to a corresponding entry
	typedef std::map<std::string, SpellcheckerEntry> SpellcheckerMap;

	/// Mapping from option name to the value of option
	typedef std::map<std::string, std::string, NocaseCmp> OptionMap;

private:
	/// The mapping from option identifiers to option content
	OptionMap options_;
	
	/// The mapping from id to spellchecker entries
	SpellcheckerMap entries_;
};

/**
 * A parse error
 */
class ParseError : public Error
{
public:
	ParseError(std::string const& what, std::string const& file,
		   int line=0, int column=0);
};

#endif // CONFIG_FILE_HH_
