/**
 * @file pipeui.cc
 * @author Pauli Virtanen
 *
 * The ispell-like pipe interface.
 */
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

#include <ctype.h>
#include <unistd.h>
#include <signal.h>

#include "config.hh"
#include "common.hh"
#include "spell.hh"
#include "tmispell.hh"

#include "pipeui.hh"


/* #undef PIPE_INPUT_DEBUG */

#ifdef PIPE_INPUT_DEBUG
std::ofstream input_log("tmispell.input",
			std::ofstream::app|std::ofstream::out);
#endif


/**
 * Start listening commands given from stdin, and print results to stdout.
 * This is compatible with the ispell -a mode.
 */
void PipeInterface::start()
{
#ifdef PIPE_INPUT_DEBUG
	input_log << "<------ Start ------>" << std::endl;
#endif

	std::ostream* out = parent_.open_output();
	
	*out << "@(#) International Ispell Version 3.1.20 compatible "
	      << PACKAGE_STRING 
	      << std::endl << std::flush;
	terse_ = false;

	filter_ = parent_.create_default_filter();
	include_depth_ = 0;

	listen_pipe(std::cin);

	delete filter_;
}

/**
 * Read commands from stdin and interpret them like ispell's -a mode.
 */
void PipeInterface::listen_pipe(std::istream& in)
{
	std::string str;
	std::ostream* out = parent_.open_output();
	
	*out << std::flush;
	while (std::getline(in, str))
	{
#ifdef PIPE_INPUT_DEBUG
		input_log << str << std::endl;
#endif
		interpret_pipe_command(parent_.from_user(str));

		*out << std::flush;

		// And stop, if user wanted so
		parent_.stop_if_needed();
	}
}

/**
 * Return the part of the string that is after the given prefix.
 * Return empty string if there is no such prefix in the string.
 */
static std::string get_string_after_prefix(Glib::ustring const& prefix,
					   Glib::ustring const& str)
{
	if (str.length() >= prefix.length() &&
	    std::equal(prefix.begin(), prefix.end(), str.begin())) 
	{
		return str.substr(prefix.length());
	}
	return Glib::ustring();
}

/**
 * Interpret the command given in the string like ispell's -a mode does,
 * and reply appropiately to it. Use the given filter, and replace it if
 * necessary.
 */
void PipeInterface::interpret_pipe_command(Glib::ustring str)
{
	if (str.empty()) return;

	// Check for commands
	switch (str[0]) 
	{
	case '*': // Add word to the personal dictionary
		str.erase(0,1);
		parent_.add_personal_word(str);
		return;
	case '&': // Insert word in lower case to the personal dictionary
		str.erase(0,1);
		tolower(str);
		parent_.add_personal_word(str);
		return;
	case '@': // Accept word in this session
		str.erase(0,1);
		parent_.add_session_word(str);
		return;
	case '#': // Save current personal dictionary
		try {
			parent_.save_personal_dictionary();
		} catch (std::string const& err) {
			// Failure here is non-fatal
			std::cerr << err << std::endl;
		}
		return;
	case '~': // Set parameters based on filename.
	{         // NOTE: This does not change the formatter!
		str.erase(0,1);
		// Nothing to do.
	} return;
	case '+': // Enter TeX mode
		delete filter_;
		filter_ = parent_.create_filter(Options::tex);
		return;
	case '-': // Exit TeX mode
		delete filter_;
		filter_ = parent_.create_filter(Options::nroff);
		return;
	case '!': // Enter terse mode
		terse_ = true;
		return;
	case '%': // Exit terse mode
		terse_ = false;
		return;
	case '^': // Spell-check rest of line
		spell_check_pipe(str, ++str.begin());
		return;
	default: break;
	}
	
	// Check for the include statement, if it is in use
	std::string const& cmd = parent_.options().pipe_include_command_;
	if (!cmd.empty())
	{
		std::string filename = get_string_after_prefix(cmd, str);
		if (!filename.empty()) 
		{
			std::ifstream file(filename.c_str());
			
			if (include_depth_ < 5) 
			{
				++include_depth_;
				listen_pipe(file);
				--include_depth_;
			}
			return;
		}
	}
	
	spell_check_pipe(str);
}

/**
 * Spell check words in the given string, and print the response in a format
 * identical to ispell's -a mode to output.
 */
void PipeInterface::spell_check_pipe(Glib::ustring const& str,
				     Glib::ustring::const_iterator sbeg)
{
	std::ostream* out = parent_.open_output();

	// Spell check words
	filter_->set_line(&str);
	filter_->reset(sbeg);

	Glib::ustring::const_iterator begin, end;
	while (filter_->get_next_word(&begin, &end))
	{
		Glib::ustring word(begin, end);
		
		if (parent_.check_word(word))
		{
			if (!terse_) *out << "*" << std::endl;
		} 
		else 
		{
			long offset = std::distance(str.begin(), begin);
			
			std::vector<Glib::ustring> suggestions; 
			parent_.get_suggestions(word, suggestions);
			
			if (suggestions.empty()) {
				*out << "# "
				     << parent_.to_user(word)
				     << " "
				     << offset
				     << std::endl;
			} else {
				*out << "& "
				     << parent_.to_user(word)
				     << " "
				     << suggestions.size()
				     << " "
				     << offset
				     << ": ";
				
				std::vector<Glib::ustring>::const_iterator i;
				for (i = suggestions.begin();
				     i != suggestions.end();
				     ++i)
				{
					if (i != suggestions.begin())
						*out << ", ";
					*out << parent_.to_user(*i);
				}
				*out << std::endl;
			}
		}
	}

	// Ispell prints also an empty line, in terse mode or not.
	*out << std::endl;
}
