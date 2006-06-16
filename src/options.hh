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
 * @file options.hh
 * @author Pauli Virtanen
 *
 * Runtime options for the program.
 */
#ifndef OPTIONS_HH_
#define OPTIONS_HH_

#include "glibmm/ustring.h"
#include <string>
#include <vector>
#include <utility>

#include "config_file.hh"

/**
 * The options controlling the program.
 */
class Options
{
public:
	/// The wanted mode of operation.
	typedef enum {
		normal, ///< Use a curses interactive front-end.
		list,   ///< Just output a list of misspelled words.
		pipe,   ///< Read commands and act according to them.
		ispell, ///< Launch the original ispell instead.
		quit    ///< Just quit.
	} Mode;

	/// A specific type of a filter.
	typedef enum {
		plain, ///< Filter like a plain text file
		tex,   ///< Filter a tex file
		nroff, ///< Filter a nroff file
		sgml   ///< Filter a sgml file
	} FilterType;
	
public:
	/// Parse given command line arguments
	Options(int argc, char* const* argv);

	/// Guess the filter to use for the given file
	static FilterType guess_file_filter(std::string const& filename);

	/// Return the arguments that should be passed to ispell.
	char const** get_ispell_argv(std::string const& prog_name) const;
	
public:
	/// Mode of operation
	Mode mode_;

	/// Are backups of files requested
	bool backups_;

	/// The include file command string
	std::string pipe_include_command_;

	/// Need to stop with SIGTSTOP after it a command is processed?
	bool sigstop_at_eol_;

	/// The identifier of the dictionary to use
	std::string dictionary_identifier_;

	/// The name of the dictionary file to be used
	std::string dictionary_;

	/// The name of the personal dictionary to be used
	std::string personal_dictionary_;

	/// The spellchecker entry to use
	SpellcheckerEntry const* spellchecker_entry_;

	/// The extra word characters to recognize
	std::vector<gunichar> extra_word_characters_;

	/// The shortest length of a words
	unsigned int legal_word_length_;

	/// Default filter type
	FilterType default_filter_;

	/// The files to check and their associated filter types
	std::vector< std::pair<std::string, FilterType> > files_;

	/// Is 7 bit ANSI display requested?
	bool ansi7_; // FIXME: No effect implemented yet

	/// Is mini menu at the bottom of the screen requested?
	bool mini_menu_; // FIXME: No effect implemented yet

	/// How many lines of context to show? -1 denotes automatic setting
	long context_lines_;

	/// Where to put the output
	std::string output_file_;

	/// The configuration file to use
	std::string config_file_;

	/// TeX command filtering
	std::string tex_command_filter_;

	/// TeX environment filtering
	std::string tex_environment_filter_;

	/// SGML attributes to check
	std::string sgml_attributes_to_check_;

	/// The user-specified encoding, if any
	std::string user_encoding_;

private:
	/// The command line parameters to pass to ispell
	std::vector<std::string> ispell_args_;

private:
	/// Print the usage information of the program
	void print_usage();
};

#endif // OPTIONS_HH_
