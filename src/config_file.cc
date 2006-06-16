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
 * @file config_file.cc
 * @author Pauli Virtanen
 *
 * Parsing the configuration file.
 */
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>

#include <cctype>

#include "i18n.hh"
#include "common.hh"
#include "regexp.hh"
#include "tmerror.hh"

#include "config_file.hh"

using std::istream;
using std::ifstream;
using std::ostringstream;
using std::map;
using std::string;

/*
 * The configuration file contains spell-checker entries and options.
 *
 * Spell checker entry format: (on one line)
 * "identifier" "library" "dictionary" "word_chars" "boundary_chars"
 *
 * Option format:
 *   <key> = <value>
 *
 * The comment character is '#' after which anything is ignored.
 * Strings may span multiple lines, if needed.
 */

/****************************************************************************/
/** @name Utility for parsing the configuration file
 ** @{
 **/

/**
 * For reading a file part by part.
 */
class ConfigFileIterator
{
public:
	/// Open a file for reading
	ConfigFileIterator(string const& file_name);

	/// Get the next quoted string, skipping no-linefeed whitespace first
	string get_next_quoted_string();

	/// Skip whitespace, optionally also linefeeds
	void skip_whitespace(bool skip_lf);

	/// Skip comments and whitespace
	void skip_comments_and_whitespace();

	/// Return the current line
	int line() const { return line_; }

	/// Match a regular expression to the current position
	bool match(RegExp& re) const {
		return re.match(buffer_, p_);
	}

	string::const_iterator begin(RegExp& re, int nth=0)
		{ return p_ + re.begin(nth); }

	string::const_iterator end(RegExp& re, int nth=0)
		{ return p_ + re.end(nth); }

	std::string sub(RegExp& re, int nth=0)
		{
			return re.sub(buffer_, nth);
		}
	
	/// Set the current position
	void position(string::const_iterator p) { p_ = p; }

	/// Check whether there is still more data
	operator bool() const { return p_ != buffer_.end(); }
	
private:
	int line_; ///< Current line
	string buffer_; ///< Buffer where file is
	string::const_iterator p_; ///< Current position in buffer
};

void ConfigFileIterator::skip_whitespace(bool skip_lf)
{
	while (p_ != buffer_.end() && std::isspace(*p_)) {
		if (*p_ == '\n') {
			++line_;
			if (!skip_lf) { ++p_; break; }
		}
		++p_;
	}
}

/**
 * Read a string, that may be quoted in single or double quotes, beginning
 * from current position, after possible whitespace. May span multiple lines.
 * If quoted, then the quoting character '\' just quotes the next character
 * as-is.
 * @return The quoted string that was found.
 */
string ConfigFileIterator::get_next_quoted_string()
{
	skip_whitespace(false);
	
	string::const_iterator str_beg; // Where the extracted substring begins
	string::const_iterator str_end; // Where the extracted substring ends

	if (p_ != buffer_.end() && (*p_ == '\'' || *p_ == '"')) {
		char quote_char = *p_;

		++p_;

		str_beg = p_;
		
		for (; p_ != buffer_.end(); ++p_) {
			if (*p_ == quote_char) {
				break;
			} else if (*p_ == '\\') {
				++p_;
				if (p_ == buffer_.end() || *p_ == '\n') {
					throw Error(_("\\ at the end of "
						      "a string"));
				}
			}
		}
		
		if (p_ != buffer_.end() && *p_ == quote_char) {
			str_end = p_;
			++p_;
		} else {
			throw Error(_("Unterminated quoted string"));
		}

	} else {
		str_beg = p_;
		while (p_ != buffer_.end() && *p_ != '\n' && !std::isspace(*p_))
			++p_;
		str_end = p_;
	}

	return string(str_beg, str_end);
}

/**
 * Skip content of lines after '#'. Also skip linefeeds and whitespace.
 */
void ConfigFileIterator::skip_comments_and_whitespace()
{
	while (p_ != buffer_.end()) {
		if (*p_ == '#') {
			while (p_ != buffer_.end() && *p_ != '\n') ++p_;
		} else if (std::isspace(*p_)) {
			skip_whitespace(true);
		} else {
			break;
		}
	}
}

/**
 * Read the given file to memory for parsing.
 */
ConfigFileIterator::ConfigFileIterator(string const& file_name)
{
	ifstream in(file_name.c_str());
	ostringstream out;

	if (!in) {
		throw Error(_("Unable to open configuration file %s"),
			    file_name.c_str());
	}

	char buf[1024];
	int readen;
	while ((readen = in.rdbuf()->sgetn(buf, 1024)) > 0)
		out.write(buf, readen);

	buffer_ = out.str();
	line_ = 1;
	p_ = buffer_.begin();
}

/** @} */


/****************************************************************************/
/** @name Parsing the configuration file
 ** @{
 **/

/**
 * Initialize a parse error message.
 */
ParseError::ParseError(std::string const& what,
		       std::string const& file, int line, int column)
	: Error("")
{
	if (line > 0 && column > 0) {
		msg_ = ssprintf(_("Parse error in file \"%s\" on line %d, "
				  "column %d: %s"),
				line, column, what.c_str());
	} else if (line > 0) {
		msg_ = ssprintf(_("Parse error in file \"%s\" on line %d: %s"),
				line, what.c_str());
	} else {
		msg_ = ssprintf(_("Parse error in file \"%s\": %s"),
				what.c_str());
	}
}

/**
 * Read a spell checker entry from config file and add the entry.
 */
static void add_spell_checker_entry(ConfigFileIterator& i,
				    ConfigFile::SpellcheckerMap& entries)
{
	string id = i.get_next_quoted_string();
	
	string library = i.get_next_quoted_string();
	string dictionary = i.get_next_quoted_string();
	string encoding = i.get_next_quoted_string();
	string lc_ctype = i.get_next_quoted_string();
	string word_chars = i.get_next_quoted_string();
	string boundary_chars = i.get_next_quoted_string();

	if (id.empty() || library.empty() || dictionary.empty() ||
	    encoding.empty()) {
		throw Error(_("Incomplete spell checker entry"));
	}
	
	entries.insert(
		ConfigFile::SpellcheckerMap::value_type(
			id,
			SpellcheckerEntry(library,
					  dictionary,
					  encoding,
					  lc_ctype,
					  word_chars,
					  boundary_chars)));
}

/**
 * Check if there is an option line at current position.
 * If so, store the key-value pair.
 * NOTE: This assumes that i is at a beginning of something, not at whitespace.
 */
static bool handle_option_lines(ConfigFileIterator& i,
				ConfigFile::OptionMap* options)
{
	static RegExp optre("^([a-zA-Z0-9_-]+)[[:space:]]*=[[:space:]]*",
			    RegExp::EXTENDED|RegExp::ICASE);

	if (i.match(optre)) {
		i.position(i.end(optre));
		string read = i.get_next_quoted_string();
		(*options)[i.sub(optre, 1)] = read;
		return true;
	} else {
		return false;
	}
}

/**
 * Read and parse configuration file.
 */
ConfigFile::ConfigFile(string const& file_name)
{
	ConfigFileIterator i(file_name);

	i.skip_comments_and_whitespace();
	while (i) {
		try {
			if (handle_option_lines(i, &options_))
				/* nothing */;
			else
				add_spell_checker_entry(i, entries_);
			
		} catch (Error const& err) {
			throw ParseError(err.what(),
					 file_name,
					 i.line());
		}
		
		i.skip_comments_and_whitespace();
	}
}

/** @} */


/** Create a new spell checker entry */
SpellcheckerEntry::SpellcheckerEntry(std::string library,
				     std::string dictionary,
				     std::string encoding,
				     std::string lc_ctype,
				     std::string word_chars,
				     std::string boundary_chars)
	: library_(library),
	  dictionary_(dictionary),
	  encoding_(encoding),
	  lc_ctype_(lc_ctype)
{
	word_chars_.insert(word_chars_.begin(), 
			   word_chars.begin(),
			   word_chars.end());
	boundary_chars_.insert(boundary_chars_.begin(), 
			       boundary_chars.begin(),
			       boundary_chars.end());
}

/**
 * Case insensitive string lexical StrictWeakOrdering.
 */
bool NocaseCmp::operator()(std::string const& s1, std::string const& s2) const
{
	std::string::const_iterator i1, i2;

	for (i1 = s1.begin(), i2 = s2.begin();
	     i1 != s1.end() && i2 != s2.end();
	     ++i1, ++i2)
	{
		int r = std::tolower(*i1) - std::tolower(*i2);
		if (r < 0)
			return true;
		else if (r > 0)
			return false;
	}
	return false;
}

/**
 * Gets a string corresponding to the given key from the given map,
 * or empty string if there is no match.
 */
SpellcheckerEntry const& ConfigFile::get(string const& id) const
{
	static SpellcheckerEntry none("", "", "", "", "", "");

	SpellcheckerMap::const_iterator i = entries_.find(id);
	if (i == entries_.end()) {
		return none;
	} else {
		return i->second;
	}
}

bool ConfigFile::has(std::string const& id) const
{
	return (entries_.find(id) != entries_.end());
}


string const& ConfigFile::get_option(string const& option_name) const
{
	static string none;

	OptionMap::const_iterator i = options_.find(option_name);
	if (i == options_.end()) {
		return none;
	} else {
		return i->second;
	}
}
