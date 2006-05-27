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
 * @file spell.cc
 * @author Pauli Virtanen
 *
 * The interface to the spell checking library.
 */
#include <string>
#include <sstream>
#include <vector>
#include <stdio.h>

#include <dlfcn.h>

#include "config.hh"
#include "common.hh"
#include "i18n.hh"
#include "spell.hh"

using std::string;
using std::ostringstream;
using std::vector;


/**
 * Load a symbol from the given library handle.
 */
static void* dl_symbol(void* handle, char const* name)
{
	void* symbol;
	
	symbol = dlsym(handle, name);
	
	if (symbol == 0) {
		throw Error(_("Unable to locate symbol %s in library"),
			    name);
	}
	
	return symbol;
}

/**
 * Opens the given spell check library and loads the given dictionary.
 * Encoding is set to latin9 by default.
 */
Spellchecker::Spellchecker(std::string const& library,
			   std::string const& dictionary,
			   std::string const& encoding)
	: initialized_(false)
{
	try {
		dl_handle_ = dlopen(library.c_str(), RTLD_NOW);

		if (dl_handle_ == 0) {
			throw Error(_("Unable to open library"));
		}
		
		init_func_ = (initvoikko_t)dl_symbol(
			dl_handle_, "voikko_init");
		check_func_ = (spell_t)dl_symbol(
			dl_handle_, "voikko_spell_cstr");
		suggest_func_ =	(suggest_t)dl_symbol(
			dl_handle_,  "voikko_suggest_cstr");
		string_option_func_ = (setsopt_t)dl_symbol(
			dl_handle_, "voikko_set_string_option");
		boolean_option_func_ = (setbopt_t)dl_symbol(
			dl_handle_, "voikko_set_bool_option");
		const char* error = init_func_(&voikkohandle, "fi_FI", 0);
		if (error != 0) 
			throw Error(_("Initializing spell checker failed"));
		boolean_option_func_(voikkohandle, VOIKKO_OPT_IGNORE_DOT, 1);
		boolean_option_func_(voikkohandle, VOIKKO_OPT_IGNORE_NUMBERS, 1);
		boolean_option_func_(voikkohandle, VOIKKO_OPT_IGNORE_UPPERCASE, 1);
		initialized_ = true;
		open_dictionary(dictionary);
		set_encoding(encoding);
	} catch (Error const& err) {
		if (dl_handle_ != 0) dlclose(dl_handle_);
		dl_handle_ = 0;
		initialized_ = false;
		throw Error("Error loading library %s: %s",
			    library.c_str(), err.what());
	}

	conv_ = new CharsetConverter("UTF-8");
}

/**
 * Terminates and unloads the spell check library.
 */
Spellchecker::~Spellchecker()
{
	dlclose(dl_handle_);
	delete conv_;
	initialized_ = false;
}

/**
 * Returns the version of the spell check library
 */
int Spellchecker::get_version()
{
	return 0;
}

/**
 * Opens the given dictionary.
 */
void Spellchecker::open_dictionary(string const& dictionary_path)
{

}

/**
 * Changes encoding.
 */
void Spellchecker::set_encoding(string const& encoding)
{
	int status;
	status = string_option_func_(voikkohandle, VOIKKO_OPT_ENCODING, encoding.c_str());
	encoding_ = encoding;
	
	if (!status)
		throw Error(_("Unable to set encoding to %s"),
			    encoding.c_str());
}


/**
 * Checks the spelling of a word.
 * @return Whether the word is correctly spelled.
 */
bool Spellchecker::check_word(Glib::ustring const& word)
{
	int status;

	std::string lword = conv_->to(word);


	status = check_func_(voikkohandle, lword.c_str());
	if (status) return true;
	else return false;
}

#define SUGGESTION_BUFFER_SIZE 1024

/**
 * Fetches correct suggestions for a misspelled word.
 */
void Spellchecker::get_suggestions(Glib::ustring const& word, 
				   vector<Glib::ustring>& suggestions)
{
	char ** vsuggestions;
	int word_count;

	std::string lword = conv_->to(word);
	vsuggestions = suggest_func_(voikkohandle, lword.c_str());
	
	word_count = 0;
	if (vsuggestions != 0) while (vsuggestions[word_count] != 0) word_count++;
	suggestions.clear();
	suggestions.reserve(word_count);

	for (int i = 0; i < word_count; i++)
	{
		std::string str(vsuggestions[i]);
		suggestions.push_back(conv_->from(str));
		free(vsuggestions[i]);
	}
	free(vsuggestions);
}
