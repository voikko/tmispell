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
 * @file tmispell.cc
 * @author Pauli Virtanen
 * 
 * An ispell-like spell checking front-end.
 *
 */
#include <string>
#include <iostream>
#include <fstream>

#include <locale.h>

#include <unistd.h>
#include <signal.h>

#include "config.hh"
#include "common.hh"
#include "i18n.hh"
#include "config_file.hh"
#include "spell.hh"
#include "options.hh"
#include "filter.hh"
#include "personal_dictionary.hh"
#include "tmispell.hh"
#include "charset.hh"

#include "ui/listui.hh"
#include "ui/pipeui.hh"
#include "ui/cursesui.hh"

#include "glibmm/convert.h"
#include "glibmm/error.h"

/**
 * Initialize and parse the command line parameters to options.
 */
IspellAlike::IspellAlike(int argc, char* const* argv) 
	: options_(argc, argv), sp_(0), user_conv_(0), out_(0)
{
}

/**
 * Uninitialize.
 */
IspellAlike::~IspellAlike()
{
	delete out_;
	delete sp_;
}

/**
 * Start the program.
 *
 * Check that if have a module for the wanted language, and launch the real
 * ispell otherwise.
 * Initialize the spell checking engine, load personal dictionary and switch
 * to an operating mode specified by the options. Save personal dictionary
 * afterwards.
 */
void IspellAlike::start()
{
	// Check if the user wants to quit right away
	if (options_.mode_ == Options::quit) return;

	// Load configuration file
	ConfigFile conffile(options_.config_file_);

	// Check that ispell was given
	std::string ispell = conffile.get_option("ispell");
	if (ispell.empty()) {
		throw Error(_("An Ispell program was not given in "
			      "the configuration file %s"),
			    options_.config_file_.c_str());
	}
	
	// Launch ispell, if so wanted
	if (options_.mode_ == Options::ispell) {
		launch_old_ispell(ispell);
		return;
	}

	// Check whether we have this language available through a module
	std::string const& dict_id = options_.dictionary_identifier_;

	// Start the old ispell, if there is no matching dictionary
	if (dict_id.empty() || !conffile.has(dict_id)) {
		launch_old_ispell(ispell);
		return;
	}

	// Set up the chosen spell checker
	options_.spellchecker_entry_ = &conffile.get(dict_id);

	// Get options from configuration file
	options_.tex_command_filter_
		= conffile.get_option("tex-command-filter");
	options_.tex_environment_filter_
		= conffile.get_option("tex-environment-filter");
	options_.sgml_attributes_to_check_
		= conffile.get_option("sgml-attributes-to-check");
	// Start spell checking engine
	try {
		sp_ = new Spellchecker(
			options_.spellchecker_entry_->get_library(), 
			options_.spellchecker_entry_->get_dictionary(),
			options_.spellchecker_entry_->get_encoding());
	} catch (Error const& err) {
		// Spellchecker failed to initialize: Spew error and try to
		// launch ispell instead.
		std::cerr << err.what() << std::endl;
		launch_old_ispell(ispell);
	}
	// Prepare encoding
	delete user_conv_;
	if (!options_.user_encoding_.empty()) {
		user_conv_ = new CharsetConverter(
			options_.user_encoding_.c_str());
	}

	// Load personal dictionary
	try {
		personal_dictionary_.load(options_.personal_dictionary_);
	} catch (Error const& err) {
	}

	// Start the wanted interface
	switch (options_.mode_)
	{
	case Options::normal: {
		CursesInterface i(*this);
		i.start();
	} break;

	case Options::list: {
		ListInterface i(*this);
		i.start();
	} break;

	case Options::pipe: {
		PipeInterface i(*this);
		i.start();
	} break;

	default:
		// This should never happen: no need to localize
		throw Error("FIXME: Mode unsupported");
	}

	// Save personal dictionary if necessary.
	if (personal_dictionary_.is_changed()) {
		try {
			personal_dictionary_.save(
				options_.personal_dictionary_);
		} catch (Error const& err) {
			// Failing to save personal dictionary is not fatal
			std::cerr << err.what() << std::endl;
			return;
		}
	}
}

/**
 * Program entry point.
 */
void throw_glib_convert_error_func(GError* gobject)
{
	throw Glib::ConvertError(gobject);
}

int main(int argc, char* const* argv)
{
	try {
		locale_init();

		{
			Glib::Error::register_init();
			GQuark eq =
				g_quark_from_static_string("g_convert_error");
			Glib::Error::register_domain(
				eq,
				&throw_glib_convert_error_func);
		}
		

		IspellAlike ispellalike(argc, argv);
		ispellalike.start();
		return 0;
	} catch (Error const& err) {
		std::cerr << err.what() << std::endl;
		return -1;
	}
}



/****************************************************************************/
/** @name Utility functions for front-ends.
 ** @{
 **/

bool IspellAlike::check_word(Glib::ustring const& str)
{
	if (str.length() < options_.legal_word_length_) {
		return true;
	}
	return sp_->check_word(str) ||
		personal_dictionary_.check_word(str) ||
		session_dictionary_.check_word(str);
}

Filter* IspellAlike::create_filter(Options::FilterType type)
{
	return Filter::new_filter(type, options_);
}

Filter* IspellAlike::create_default_filter()
{
	return create_filter(options_.default_filter_);
}

std::ostream* IspellAlike::open_output()
{
	if (!options_.output_file_.empty()) {
		if (out_ == 0) {
			out_ =new std::ofstream(options_.output_file_.c_str());
			if (!(*out_)) {
				throw Error(_("Unable to write to file %s"),
					    options_.output_file_.c_str());
			}
		}
		return out_;
	} else {
		return &std::cout;
	}
}

void IspellAlike::stop_if_needed()
{
	if (options_.sigstop_at_eol_) {
		kill(getpid(), SIGTSTP);
	}
}

void IspellAlike::add_personal_word(Glib::ustring const& str)
{
	personal_dictionary_.add_word(str);
}

void IspellAlike::add_session_word(Glib::ustring const& str)
{
	session_dictionary_.add_word(str);
}

void IspellAlike::save_personal_dictionary()
{
	personal_dictionary_.save(options_.personal_dictionary_);
}

void IspellAlike::launch_old_ispell(std::string const& ispell)
{
	execv(ispell.c_str(), 
	      const_cast<char* const*>(options_.get_ispell_argv(ispell)));
	throw Error("Unable to start the ispell program (%s)",
		    ispell.c_str());
}

/** @} */
