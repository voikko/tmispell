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
 * @file tmispell.hh
 * @author Pauli Virtanen
 *
 * An ispell-like spell checking front-end.
 */
#ifndef TMISPELL_HH_
#define TMISPELL_HH_

#include <iosfwd>
#include <string>

#include "spell.hh"
#include "options.hh"
#include "filter.hh"
#include "personal_dictionary.hh"

#include "glibmm/ustring.h"

/**
 * A ispell-like spell checker
 */
class IspellAlike
{
public:
	/// Initialize a spell checker with the given command line parameters.
	IspellAlike(int argc, char* const* argv);
	
	/// Destroy this spell checker
	~IspellAlike();

	/// Start this spell checker
	void start();

public:
	/*
	 * Spell checking
	 */

	/// Check if the given word is spelled correctly
	bool check_word(Glib::ustring const& str);

	void get_suggestions(Glib::ustring const& str,
			     std::vector<Glib::ustring>& suggestions)
		{ sp_->get_suggestions(str, suggestions); }

	/*
	 * Personal and session dictionaries
	 */

	/// Add a word to the personal dictionary
	void add_personal_word(Glib::ustring const& str);

	/// Add a word to the session dictionary
	void add_session_word(Glib::ustring const& str);

	/// Save the personal dictionary
	void save_personal_dictionary();

	/*
	 * Convert input to or from user-specified encoding
	 */

	/// Convert string from user-specified encoding to UTF-8
	Glib::ustring from_user(std::string const& str)
		{return user_conv_ ? user_conv_->from(str) : from_locale(str);}

	/// Convert string from UTF-8 to user-specified encoding
	std::string to_user(Glib::ustring const& str)
		{return user_conv_ ? user_conv_->to(str) : to_locale(str);}


	/*
	 * Convert input to or from the locale-specified encoding.
	 */

	/// Convert string from locale-specified encoding to UTF-8
	Glib::ustring from_locale(std::string const& str)
		{ return CharsetConverter::locale().from(str); }

	/// Convert string from UTF-8 to locale-specified encoding
	std::string to_locale(Glib::ustring const& str)
		{ return CharsetConverter::locale().to(str); }

	/*
	 * Filters
	 */
	
	/// Return a new filter object of default type
	Filter* create_default_filter();

	/// Return a new filter object of given type
	Filter* create_filter(Options::FilterType type);

	/*
	 * Producing output
	 */

	/// Opens the output channel if not already opened.
	std::ostream* open_output();

	/// Send SIGTSTOP to itself, if needed
	void stop_if_needed();

	
	/// Get option information
	Options const& options() const { return options_; }
	
private:
	/// Launch the real ispell program instead
	void launch_old_ispell(std::string const& ispell);

private:
	/// Options supplied by the user and defaults
	Options options_;

	/// The user's personal dictionary of accepted words
	PersonalDictionary personal_dictionary_;

	/// Additional words accepted in this session
	PersonalDictionary session_dictionary_;

	/// The spell checker
	Spellchecker* sp_;

	/// The converter for the user-specified encoding
	CharsetConverter* user_conv_;

	/// Output channel
	std::ostream* out_;
};

#endif // TMISPELL_HH_
