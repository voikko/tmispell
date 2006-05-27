/**
 * @file filter.cc
 * @author Pauli Virtanen
 *
 * Extracting the parts from input that require spell checking.
 *
 * FIXME: The filter architecture needs an overhaul. Doing filtering
 * FIXME: line orientedly is stupid: it should clearly be stream-based.
 * FIXME: Currently there is some overlapping functionality in cursesui
 * FIXME: (namely Context), which is somewhat stream-based. It uses
 * FIXME: filter, though.
 */

#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <deque>
#include <map>
#include <set>

#include "glibmm/ustring.h"
#include "glibmm/unicode.h"
#include "regexp.hh"
#include "filter.hh"
#include "options.hh"

/****************************************************************************/
/** @name Filtering plain text
 ** @{
 **/

/** strncmp. Here, because g++ and g++-3.2 differ too much */
static bool is_string_at(Glib::ustring::const_iterator beg,
			 Glib::ustring::const_iterator end,
			 Glib::ustring const& str)
{
	Glib::ustring::const_iterator p = str.begin();
	while (beg != end && p != str.end()) {
		if (*beg != *p) return false;
		++beg;
		++p;
	}
	return p == str.end();
}

/**
 * Plain filter. Just splits words.
 * FIXME: Relies on the fact that the locale is set properly.
 */
class PlainFilter : public Filter
{
public:
	/// Initialize the word character table.
	PlainFilter(Options const& options);
	virtual ~PlainFilter() {}
	
	/// Return to a specified position in a line.
	virtual void reset(Glib::ustring::const_iterator pos){ pos_ = pos; }

	/// Set a new line.
	virtual void set_line(Glib::ustring const* line)
		{
			Filter::set_line(line);
			pos_ = line_->begin();
		}

	/// Get next whole word from the line.
	virtual bool get_next_word(Glib::ustring::const_iterator* found_begin,
				   Glib::ustring::const_iterator* found_end);

protected:
	/** Is the given character a part of a word? */
	bool is_word_char(gunichar c) const
		{ return Glib::Unicode::isalpha(c) ||
			  word_characters_.find(c) != word_characters_.end(); }

	/** 
	 * Is the given character a part of a word when between word
	 * characters? 
	 */
	bool is_boundary_char(gunichar c) const
		{ return boundary_characters_.find(c)
			  != boundary_characters_.end(); }

	/** Is the current position valid */
	bool is_pos_valid() const { return pos_ != line_->end(); }

	/** Is *pos_ a boundary? */
	bool is_at_boundary() const
		{ return is_pos_valid() && is_boundary_char(*pos_); }

	/** Is *pos_ a word character? */
	bool is_at_word() const
		{ return is_pos_valid() && is_word_char(*pos_); }

	/** Is at given character */
	bool is_at(gunichar ch) const
		{ return (is_pos_valid() && *pos_ == ch); }

	/// Is there a str at current pos_?
	bool is_at(Glib::ustring const& str)
		{ return is_string_at(pos_, line_->end(), str); }
	
	/// Is there a str at current pos_?
	bool is_at(char const* str)
		{ return is_at(Glib::ustring(str)); }
	
	/** Skip word characters and boundary characters inside words */
	void skip_over_word();

	/** Skip whitespace */
	void skip_whitespace()
		{ while (is_pos_valid() && Glib::Unicode::isspace(*pos_)) ++pos_; }

	/** Skip non-whitespace */
	void skip_non_whitespace()
		{ while (is_pos_valid() && !Glib::Unicode::isspace(*pos_)) ++pos_; }
	
	/** Skip over word characters */
	void skip_word_characters()
		{ while (is_at_word()) ++pos_; }

	/** Skip over non-word characters */
	void skip_non_word_characters()
		{ while (is_pos_valid() && !is_word_char(*pos_)) ++pos_;}

	/** Skip over n characters, if possible */
	void skip_n(int n)
		{ for (; is_pos_valid() && n > 0; --n) ++pos_; }

	/** The current position in the current line */
	Glib::ustring::const_iterator pos_;

private:
	/** Add characters to word character lookup table */
	void add_word_characters(std::vector<gunichar> const& chars)
		{
			std::vector<gunichar>::const_iterator i;
			for (i = chars.begin(); i != chars.end(); ++i) {
				word_characters_.insert(*i);
			}
		}

	/** Add characters to boundary character lookup table */
	void add_boundary_characters(std::vector<gunichar> const &chars)
		{
			std::vector<gunichar>::const_iterator i;
			for (i = chars.begin(); i != chars.end(); ++i) {
				boundary_characters_.insert(*i);
			}
		}

	/** Lookup table of word characters */
	std::set<gunichar> word_characters_;

	/** Lookup table of boundary characters */
	std::set<gunichar> boundary_characters_;
};

PlainFilter::PlainFilter(Options const& options) 
{
	add_word_characters(options.extra_word_characters_);
	add_word_characters(options.spellchecker_entry_->get_word_chars());
	add_boundary_characters(
		options.spellchecker_entry_->get_boundary_chars());
}

void PlainFilter::skip_over_word()
{
	while (1) {
		skip_word_characters();

		if (is_at_boundary()) {
			++pos_;
			if (!is_at_word()) {
				--pos_;
				break;
			}
		} else {
			break;
		}
	}
}

bool PlainFilter::get_next_word(Glib::ustring::const_iterator* found_begin,
				Glib::ustring::const_iterator* found_end)
{
	skip_non_word_characters();
	*found_begin = pos_;
	skip_over_word();
	*found_end = pos_;
	
	if (*found_begin != *found_end) {
		return true;
	} else {
		return false;
	}
}

/** @} */


/****************************************************************************/
/** @name Filtering TeX
 **
 ** That is:
 **  * Skipping command names.
 **  * Skipping specific command parameters of known commands.
 **  * Skipping math.
 **
 ** This is accomplished by really parsing the TeX file, so the following
 ** part is complex.
 **
 ** There should be some amount of toleration for malformed input.
 **
 ** This was generally inspired by Aspell's TeX filter.
 **
 ** @{
 **/

/* The idea here was inspired by Aspell's TeX filter. Thanks to its author. */

/**
 * TeX filter. Uses plainfilter, but skips words that should be skipped.
 *
 * Note that internal state is sticky: the filter assumes that you feed
 * data to it as it would appear in a TeX file. The reset command may
 * be used if it does not cause a control structure to be skipped.
 *
 * How it works:
 * ============
 *
 * Tokens of type \commandname[optparam1]{param1} are called commands.
 * We keep track which parameter of which command we are currently in.
 *
 * TeXFilter::stack_.front() contains info about current command name
 * and our position in its parameter list. Info about commands is in stack
 * in nesting order.
 *
 * \begin{name} makes a special item (with name "name" and environment flag)
 * to be pushed to stack before \begin. \end then pops this item from stack.
 * This is done to keep track of e.g. math mode. Also $ and $$ are handled
 * similarry, but this time pop is done instead of push if top() item is
 * "$" or "$$".
 *
 * Since we don't really know how many parameters a command takes,
 * TeXFilter::Command has a flag waiting_param_ that tells us whether
 * we are outside a parameter. If something other than '{' or '[' comes
 * next, the command is popped from the stack.
 *
 * We base decision of whether to spell check a word on
 *   - a list of known commands with known parameter lists
 *   - a known list of environments that should not be spell checked
 */
class TeXFilter : public PlainFilter
{
public:
	/// Initialize the word character table.
	TeXFilter(Options const& options);
	virtual ~TeXFilter() {}

	/// Set a new line.
	virtual void set_line(Glib::ustring const* line)
		{ PlainFilter::set_line(line); parse_line_change(); }
	
	/// Get next whole word from the line.
	virtual bool get_next_word(Glib::ustring::const_iterator* found_begin,
				   Glib::ustring::const_iterator* found_end);

private:
	/// Notify the parser that a line has changed
	void parse_line_change();

	/// Type of a parameter for a command
	typedef enum {
		nocheck,     ///< Required parameter, do not spell check
		check,       ///< Required parameter, spell check
		opt_nocheck, ///< Optional parameter, do not spell check
		opt_check    ///< Optional parameter, spell check
	} ParamType;

	/// A parameter list
	typedef std::vector<ParamType> Params;

	/**
	 * Keep track where we are in the parameter list of a command,
	 * or denote an environment.
	 */
	struct Command
	{
		/// Create a command with given parameter list
		Command(Glib::ustring name, Params const* parms)
			: name_(name)
			{
				if (parms) {
					cur_ = parms->begin();
					end_ = parms->end();
				} else {
					cur_ = dummy_.begin();
					end_ = dummy_.end();
				}
				waiting_param_ = true;
				is_environment_ = false;
			}

		/// Create an environment
		static Command env(Glib::ustring name)
			{
				Command cmd(name, 0);
				cmd.is_environment_ = true;
				cmd.waiting_param_ = false;
				return cmd;
			}

		/// The name of this command or environment
		Glib::ustring name_;

		/// The current position in parameter list
		Params::const_iterator cur_;

		/// The end of parameter list
		Params::const_iterator end_;

		/// Is this command currently waiting for a parameter
		bool waiting_param_;

		/// Is this actually an environment
		bool is_environment_;

		/// Are there no more known parameters
		bool finished() { return cur_ == end_; }

		/// Is a parameter optional
		static bool is_opt(ParamType type) {
			return (type == opt_nocheck || type == opt_check);
		}

		/**
		 * Advance a step in the parameter list
		 * @param opt  Is the step an optional parameter?
		 */
		void advance_param(bool opt)
			{
				while (!finished()) {
					// If the param is of correct type
					if (opt == is_opt(*cur_)) {
						++cur_;
						break;
					}
					// This param to advance is not
					// optional, so skip optional params
					// in list.
					else if (!opt && is_opt(*cur_)) {
						++cur_;
					}
					// Malformed input. Treat rest as
					// unknown.
					else {
						cur_ = end_;
					}
				}
			}

		/// Something to return if nothing found
		static Params dummy_;
	};

	/// Are we currently in an environment (e.g. math) that we should skip?
	bool in_skippable_environment();

	/// Handle beginning of environment. (pos_ at {param1})
	void begin_environment();

	/// Handle end of environment. (pos_ at {param1})
	void end_environment();

	/// Discard commands that are still waiting for more parameters
	void discard_waiting_commands();

	/// Lookup known parameter list for a command
	Params const* lookup_cmd_params(Glib::ustring const& cmd);

	/// Return the topmost item in state stack
	Command& top() {
		static Command dummy("", 0);
		if (stack_.empty()) return dummy;
		return stack_.front();
	}
	/// Push a command to stack
	void push(Command const& cmd) { stack_.push_front(cmd); }
	/// Pop a command from stack
	void pop() { stack_.pop_front(); }
	/// Push an environment to stack
	void push_env(Glib::ustring name);
	/// Pop an environment from stack
	void pop_env(Glib::ustring name);

	/// Are we currently in comment?
	bool in_comment_;

	/// State stack
	std::deque<Command> stack_;

	/// Map from command names to information about their parameters
	std::map< Glib::ustring, Params > cmd_params_;

	/// Environments to skip
	std::map<Glib::ustring, bool> skip_environment_;
};

/// A dummy command: to return when nothing else found
TeXFilter::Params TeXFilter::Command::dummy_;

/**
 * Load whitespace-separated string to a map. (Set values to true.)
 */
static void load_ws_separated_string_to_map(Glib::ustring const& str,
					    std::map<Glib::ustring, bool>* mp)
{
	Glib::ustring::const_iterator p = str.begin();
	
	while (p != str.end()) {
		while (p != str.end() && Glib::Unicode::isspace(*p)) ++p;

		Glib::ustring::const_iterator beg = p;
		while (p != str.end() && !Glib::Unicode::isspace(*p)) ++p;
		Glib::ustring name(beg, p);

		if (!name.empty()) {
			(*mp)[name] = true;
		}
	}
}

/**
 * Initialize a TeXFilter: init the known command parameter map.
 * Also init underlying PlainFilter.
 */
TeXFilter::TeXFilter(Options const& options)
	: PlainFilter(options)
{
	cmd_params_.clear();
	skip_environment_.clear();

	/* Parse the command parameter info string.
	 * Syntax: <command_name> <parameters>, ...
	 * 
	 * Parameters: a string of characters 'p', 'P', 'o', 'O'.
	 * Denote parameters and optional parameters in order.
	 *   'p': Parameter, do spell check.
	 *   'P': Parameter, do not spell check.
	 *   'o': Optional parameter, do spell check.
	 *   'O': Optional parameter, do not spell check.
	 */
	char const* p = options.tex_command_filter_.c_str();
	
	while (*p != '\0') {
		while (Glib::Unicode::isspace(*p)) ++p;

		char const* beg = p;
		while (*p != '\0' && !Glib::Unicode::isspace(*p)) ++p;
		Glib::ustring name = Glib::ustring(beg, p - beg);

		while (Glib::Unicode::isspace(*p)) ++p;

		Params parms;
		while (*p != '\0' && *p != ',') {
			switch (*p) {
			case 'P': parms.push_back(check); break;
			case 'p': parms.push_back(nocheck); break;
			case 'O': parms.push_back(opt_check); break;
			case 'o': parms.push_back(opt_nocheck); break;
			}
			++p;
		}

		// NOTE: Handle begin and end specially: do not
		//       include the first parameter.
		if (name == "begin" || name == "end") {
			if (!parms.empty())
				parms.erase(parms.begin());
		}
		
		cmd_params_[name] = parms;

		if (*p == ',') ++p; // Skip the ,
	}

	/* Parse the environment filter string.
	 * It contains environments to be skipped.
	 */
	load_ws_separated_string_to_map(options.tex_environment_filter_,
					&skip_environment_);
}

/**
 * Look up the parameter list for a command with given name.
 * @return Pointer to parameter list, or 0 if none found.
 */
TeXFilter::Params const* TeXFilter::lookup_cmd_params(Glib::ustring const& cmd)
{
	std::map< Glib::ustring, Params >::const_iterator i;

	i = cmd_params_.find(cmd);
	if (i == cmd_params_.end()) {
		return 0;
	} else {
		return &(i->second);
	}
}

/**
 * Tell the parser that a line has changed. In effect, end comment.
 */
void TeXFilter::parse_line_change()
{
	in_comment_ = false;
}

/**
 * Return whether we are in an environment (e.g. math) that should be skipped
 * wholly.
 */
bool TeXFilter::in_skippable_environment()
{
	std::deque<Command>::const_iterator i;
	for (i = stack_.begin(); i != stack_.end(); ++i) {
		if (i->is_environment_ &&
		    skip_environment_.find(i->name_)!=skip_environment_.end())
		{
			return true;
		}
	}
	return false;
}

/**
 * Get the next word in input.
 *
 * This function:
 *   - Parses commands (e.g. \commandname[optparam1]{param2} )
 *     and keeps track which parameter we currently are in.
 *   - Decides (by environment, command and param number) whether
 *     word should be considered for spell checking.
 *   - Comments are always spell checked.
 */
bool TeXFilter::get_next_word(Glib::ustring::const_iterator* found_begin,
			      Glib::ustring::const_iterator* found_end)
{
	if (in_comment_) {
		// Comments span to end of line, but no use checking them
		return false;
		//return PlainFilter::get_next_word(found_begin,
		//				  found_end);
	}

	skip_whitespace();
	
	while (is_pos_valid()) {
		// We are at the beginning of a new command
		if (*pos_ == '\\') {
			discard_waiting_commands();

			// Note that this regexp strips the possible *
			// from the end of a command, so only the base
			// entry needs to exist in tex_commands.
			static RegExp cmdre("^\\\\([@a-zA-Z0-9]+)\\*?",
					    RegExp::EXTENDED);
			
			if (cmdre.match(*line_, pos_) &&
			    cmdre.begin(1) != cmdre.end(1)) {
				// We assume cmdre contains chars in the
				// ASCII subset of UTF-8!
				std::advance(pos_, cmdre.end(0));

				// A beginning of a command: extract name
				// and parameter spec
				Glib::ustring name(cmdre.sub(*line_, 1));
				Params const* params = lookup_cmd_params(name);
				
				// Environments receive special handling
				if (name == "begin")
					begin_environment();
				else if (name == "end")
					end_environment();

				// Environment commands are in stack
				// atop their environment.
				push(Command(name, params));
			} else {
				// Something else
				std::advance(pos_, 2);
			}
		}
		// We are at the beginning of a comment
		else if (*pos_ == '%') {
			++pos_;
			in_comment_ = true;
			return PlainFilter::get_next_word(found_begin,
							  found_end);
		}
		// We are at the beginning of a parameter for a command
		else if (*pos_ == '{') {
			++pos_;
			top().waiting_param_ = false;
		}
		// We are at the end of a parameter for a command
		else if (*pos_ == '}') {
			++pos_;

			discard_waiting_commands();
			
			// Start waiting for the next parameter.
			top().waiting_param_ = true;
			top().advance_param(false);
		}
		// We are at the beginning of an optional parameter for a cmd
		else if (*pos_ == '[') {
			++pos_;
			top().waiting_param_ = false;
		}
		// We are at the end of an optional parameter for a cmd
		else if (*pos_ == ']') {
			++pos_;

			discard_waiting_commands();
			
			top().waiting_param_ = true;
			top().advance_param(true);
		}
		// There is something else here: parameter content or end
		// of parameter list.
		else {
			discard_waiting_commands();
			
			// Then check whether there is a word to spell check
			if (is_at_word()) {
				if (!in_skippable_environment() &&
				    (top().finished() || // check unknown parms
				     *top().cur_ == check ||
				     *top().cur_ == opt_check)) {
					return PlainFilter::get_next_word(
						found_begin, found_end);
				} else {
					skip_over_word();
				}
			}
			// Then check for math $ or $$
			else if (*pos_ == '$') {
				++pos_;
				if (is_at('$')) ++pos_;
				
				if (top().is_environment_ && top().name_=="$")
					pop_env("$");
				else
					push_env("$");
			}
			// This is something else.
			else {
				++pos_;
			}
		}

		skip_whitespace();
	}
	return false;
}

/**
 * Pop the commands from stack that are waiting for more parameters.
 * This should be called before something other than a parameter (i.e. {..})
 * is read.
 */
void TeXFilter::discard_waiting_commands()
{
	// Tolerate malformed input: pop environments if something is
	// waiting for a parameter below them.
	while (stack_.size() > 1 && top().is_environment_) {
		std::deque<Command>::const_iterator i;
		for (i = stack_.begin(); i != stack_.end(); ++i) {
			if (!i->is_environment_) break;
		}
		if (i != stack_.end() && i->waiting_param_) {
			pop();
		} else {
			break;
		}
	}

	// Pop all commands from stack that are still
	// waiting for more parameters. Their parameter
	// list ends here.
	while (!stack_.empty() && top().waiting_param_ &&
	       !top().is_environment_)
	{
		pop();
	}
}

/**
 * Push an environment to the state stack.
 */
void TeXFilter::push_env(Glib::ustring name)
{
	push(Command::env(name));
}

/**
 * Pop an enviromnent from the state stack.
 * To tolerate malformed input, also look if the environment
 * exists deeper in the stack.
 */
void TeXFilter::pop_env(Glib::ustring name)
{
	// Pop last environment with given name. If no such found, just
	// pop the last environment.
	std::deque<Command>::iterator i;
	for (i = stack_.begin(); i != stack_.end(); ++i) {
		if (i->is_environment_ && i->name_ == name) {
			stack_.erase(i);
			return;
		}
	}
	for (i = stack_.begin(); i != stack_.end(); ++i) {
		if (i->is_environment_) {
			stack_.erase(i);
			return;
		}
	}
}

/// A regexp to extract a name for an environment from a \begin or \end.
static RegExp tex_envre("^\\{([a-zA-Z0-9]+)\\*?\\}", RegExp::EXTENDED);

/**
 * Handle the beginning of an environment: extract name and push to stack.
 * This should be called after \begin encountered.
 */
void TeXFilter::begin_environment()
{
	if (tex_envre.match(*line_, pos_)) {
		// We assume tex_envre contains chars in the ASCII subset of
		// UTF-8!
		std::advance(pos_, tex_envre.end(0));
		
		push_env(tex_envre.sub(*line_, 1));
	}
}

/**
 * Handle the end of an environment: extract name and pop from stack.
 * This should be called after \end encountered.
 */
void TeXFilter::end_environment()
{
	if (tex_envre.match(*line_, pos_)) {
		// We assume tex_envre contains chars in the ASCII subset of
		// UTF-8!
		std::advance(pos_, tex_envre.end(0));
		
		pop_env(tex_envre.sub(*line_, 1));
	}
}

/** @} */


/****************************************************************************/
/** @name Filtering SGML-kin
 ** @{
 **/

/**
 * Filter for SGML, HTML, XML and the like.
 */
class SGMLFilter : public PlainFilter
{
public:
	/// Initialize the word character table.
	SGMLFilter(Options const& options);
	virtual ~SGMLFilter() {}

	/// Get next whole word from the line.
	virtual bool get_next_word(Glib::ustring::const_iterator* found_begin,
				   Glib::ustring::const_iterator* found_end);

private:
	/// Check if we are currently in a good attribute
	bool in_good_attribute();

	/// Are we currently in markup
	bool in_markup_;

	/// What kind of quote we are currently in?
	unsigned char quote_char_;

	/// The name of the current tag
	Glib::ustring tag_name_;

	/// The name of the current attribute
	Glib::ustring attribute_name_;

	/// Which attributes to spell check
	std::map<Glib::ustring, bool> attributes_to_check_;
};

/**
 * Initialize SGMLFilter. Load a list of checkable attributes.
 */
SGMLFilter::SGMLFilter(Options const& options)
	: PlainFilter(options),
	  in_markup_(false), quote_char_(0), tag_name_(), attribute_name_()
{
	// The format of "sgml-attributes-to-check" option is just
	// a whitespace separated list of attributes
	load_ws_separated_string_to_map(options.sgml_attributes_to_check_,
					&attributes_to_check_);
}

/**
 * Check whether we want to spell check this attribute
 * That is, is it in attributes_to_check_?
 */
bool SGMLFilter::in_good_attribute()
{
	return attribute_name_.empty() || // Malformed input?
		(attributes_to_check_.find(attribute_name_)
		 != attributes_to_check_.end());
}

/**
 * Return the next word to be spell checked.
 * Skip markup and spell check only attributes appearing in
 * attributes_to_check_.
 *
 * FIXME: We need an entity encoder and decoder, but it requires changes
 * FIXME: to the filter architecture. (The changes are on TODO list.)
 */
bool SGMLFilter::get_next_word(Glib::ustring::const_iterator* found_begin,
			       Glib::ustring::const_iterator* found_end)
{
	skip_whitespace();
	while (is_pos_valid())
	{
		
		// Handle the beginning of a tag
		if (*pos_ == '<') {
			++pos_;

			if (is_at('/')) // End tag
				++pos_;

			// Get the tag name
			Glib::ustring::const_iterator beg = pos_;
			while (is_pos_valid() && Glib::Unicode::isgraph(*pos_)
			       && *pos_ != '>') ++pos_;
			
			if (!in_markup_) {
				tag_name_.assign(beg, pos_);
			} else {
				// Malformed input.
				tag_name_.resize(0);
			}
			in_markup_ = true;
			attribute_name_.resize(0);
		}
		// Handle the end of a tag
		else if (*pos_ == '>') {
			++pos_;
			in_markup_ = false;
			tag_name_.resize(0);
			attribute_name_.resize(0);
		}
		// Handle a SGML shortened tag
		else if (*pos_ == '/' && in_markup_ && quote_char_ == 0) {
			++pos_;
			tag_name_.resize(0);
			attribute_name_.resize(0);
			in_markup_ = false;
		}
		// Handle a quote
		else if ((*pos_ == '"' || *pos_ == '\'') &&
			 (quote_char_ == 0 || quote_char_ == *pos_)) {
			
			if (quote_char_ != 0) {
				quote_char_ = 0; // End of quote
			} else {
				quote_char_ = *pos_; // Begin quote
			}
			++pos_;
		}
		// Handle an attribute name
		else if (in_markup_ && quote_char_ == 0 &&
			 isalnum(*pos_)) {
			// Get the attribute name
			Glib::ustring::const_iterator beg = pos_;
			while (is_pos_valid() && Glib::Unicode::isgraph(*pos_) &&
			       *pos_ != '=' && *pos_ != '/' && *pos_ != '"' &&
			       *pos_ != '\'') ++pos_;
			attribute_name_.assign(beg, pos_);
		}
		// Skip entities
		else if ((!in_markup_ || quote_char_ != 0) && *pos_ == '&') {
			++pos_;
			while (is_pos_valid() && isalnum(*pos_) &&
			       *pos_ != ';') ++pos_;
			if (is_at(';')) ++pos_;
		}
		// We are at a word
		else if (is_at_word()) {

			// Decide whether whe should spell check this word
			if (!in_markup_ ||
			    (quote_char_ != 0 && in_good_attribute())) {
				return PlainFilter::get_next_word(found_begin,
								  found_end);
			} else {
				++pos_;
				while (is_pos_valid() && isalnum(*pos_))
					++pos_;
			}
		}
		// We are somewhere else: just skip it
		else {
			++pos_;
		}

		skip_whitespace();
	}
	return false;
}

/** @} */


/****************************************************************************/
/** @name Filtering *ROFF-kin
 ** @{
 **/

/* The rules here are just copied from Ispell. Thanks to its authors.
 * FIXME: However, the filter is not very complete.
 */

/**
 * Filter for troff etc.
 */
class NroffFilter : public PlainFilter
{
public:
	/// Initialize the word character table.
	NroffFilter(Options const& options);
	virtual ~NroffFilter() {}

	/// Set a new line.
	virtual void set_line(Glib::ustring const* line)
		{ PlainFilter::set_line(line); line_changed_ = true; }
	
	/// Get next whole word from the line.
	virtual bool get_next_word(Glib::ustring::const_iterator* found_begin,
				   Glib::ustring::const_iterator* found_end);
	
private:
	bool is_at_request(Glib::ustring const& request,
			   Glib::ustring::const_iterator* end);

	/// Has the line changed
	bool line_changed_;
};

NroffFilter::NroffFilter(Options const& options)
	: PlainFilter(options), line_changed_(true)
{
}

bool NroffFilter::is_at_request(Glib::ustring const& request,
				Glib::ustring::const_iterator* end)
{
	Glib::ustring::const_iterator i = pos_;
	bool at_begin;

	while (i != line_->end() && Glib::Unicode::isspace(*i)) ++i;
	at_begin = line_changed_ && (i == pos_);
	
	if (i != line_->end() && *i == '.') {
		++i;
		if (at_begin) {
			// .  ds, for example
			while (i != line_->end() && Glib::Unicode::isspace(*i)) ++i;
		}
		*end = i;
		std::advance(*end, request.size());
		return is_string_at(i, line_->end(), request);
	} else {
		return false;
	}
}

/**
 * Try to do some *roff parsing
 */
bool NroffFilter::get_next_word(Glib::ustring::const_iterator* found_begin,
				Glib::ustring::const_iterator* found_end)
{
	if (line_changed_ && is_pos_valid() && *pos_ == '.') {
		Glib::ustring::const_iterator p;

		// Skip over if
		if (is_at_request("if t", &p) || is_at_request("if n", &p) ||
		    is_at_request("el ", &p) || is_at_request("ie ", &p)) {
			pos_ = p;
			skip_whitespace();
			line_changed_ = false;
		}
		
		// Skip over .ds XX or .nr XX
		if (is_at_request("ds ", &p) || is_at_request("de ", &p) ||
		    is_at_request("nr ", &p)) {
			pos_ = p;
			skip_whitespace();
			skip_non_whitespace();
			skip_whitespace();
			line_changed_ = false;
		}
		
		// Skip over the following formatter command, if any
		if (is_at_request("", &p)) {
			pos_ = p;
			skip_non_whitespace();
		}
		line_changed_ = false;
	}

	skip_whitespace();
	while (is_pos_valid()) {
		// *roff escape sequence
		if (*pos_ == '\\') {
			// Font change \f(XY or \fX
			if (is_at("\\f")) {
				std::advance(pos_, 2);
				if (is_at('('))
					skip_n(3);
				else
					skip_n(1);
			}
			// Font change end \f)
			else if (is_at("\\f)")) {
				std::advance(pos_, 3);
			}
			// Size change
			else if (is_at("\\s")) {
				std::advance(pos_, 2);
				if (is_at('+') || is_at('-')) {
					++pos_;
				}
				skip_n(1); // Should be a digit anyway
				if (is_pos_valid() && isdigit(*pos_))
					++pos_;
			}
			// Extended chars
			else if (is_at("\\(")) { // \(XX
				std::advance(pos_, 2);
				skip_n(2);
			}
			else if (is_at("\\*")) { // \\*
				std::advance(pos_, 2);
				if (is_at('(')) // \*(XX
					skip_n(3);
			}
			// Whatever...
			else {
				++pos_;
			}
		}
		// Word, try and get it
		else if (is_at_word()) {
			return PlainFilter::get_next_word(found_begin,
							  found_end);
		}
		// Something else: skip it
		else {
			++pos_;
		}
		skip_whitespace();
	}
	return false;
}

/** @} */


/****************************************************************************/
/** @name Filter selection
 ** @{
 **/

/**
 * Return a filter of the given type, using the given options.
 */
Filter* Filter::new_filter(Options::FilterType type, Options const& options)
{
	switch (type)
	{
	case Options::plain: return new PlainFilter(options);
	case Options::nroff: return new NroffFilter(options);
	case Options::tex: return new TeXFilter(options);
	case Options::sgml: return new SGMLFilter(options);
	default:
		//std::cerr << "Requested filter not implemented" << std::endl;
		break;
	}

	return new PlainFilter(options);
}

/** @} */
