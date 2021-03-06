/* Copyright (C) Pauli Virtanen
 *               2006 Harri Pitkänen <hatapitk@iki.fi>
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
 * @file spell.hh
 * @author Pauli Virtanen
 *
 * The interface to the spell checking library.
 */
#ifndef SPELL_HH_
#define SPELL_HH_

#include <string>
#include <vector>
#include <libvoikko/voikko.h>

#include "glibmm/ustring.h"
#include "glibmm/unicode.h"

#include "charset.hh"


/**
 * The spelling checker.
 */
class Spellchecker
{
public:
	/// Load the given dynamic library and use the given dictionary
	Spellchecker(std::string const& library,
		     std::string const& dictionary,
		     std::string const& encoding="latin9");

	/// Deinitialize the spell checker
	~Spellchecker();

	/// Return the version of the library
	int get_version();
	
	/// Open the given dictionary
	void open_dictionary(std::string const& dictionary_path);

	/// Set the input and output encoding
	void set_encoding(std::string const& encoding);

	/// Check the spelling of a word
	bool check_word(Glib::ustring const& word);
	bool check_word(Glib::ustring::const_iterator begin,
			Glib::ustring::const_iterator end)
		{ return check_word(Glib::ustring(begin, end)); }

	/// Produce suggestions for a misspelled word
	void get_suggestions(Glib::ustring const& word,
			     std::vector<Glib::ustring>& suggestions);
	void get_suggestions(Glib::ustring::const_iterator begin,
			     Glib::ustring::const_iterator end,
			     std::vector<Glib::ustring>& suggestions)
		{ get_suggestions(Glib::ustring(begin, end), suggestions); }

private:

	int			voikkohandle;

	/// Is the loaded library properly initialized
	bool			initialized_;

	/// The dictionary file the library uses
	std::string             dictionary_file_;

	/// The library file to load
	std::string             library_file_;

	/// The encoding
	std::string             encoding_;

	/// Character set converter
	CharsetConverter*	conv_;
};

#endif // SPELL_HH_
