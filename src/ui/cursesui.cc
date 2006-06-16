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
 * @file cursesui.cc
 * @author Pauli Virtanen
 *
 * An ispell-like curses text interface.
 * Quite a direct cloning of the Ispell interface.
 */
#include <string>
#include <fstream>
#include <algorithm>
#include <vector>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <curses.h>
#include <signal.h>

#include "i18n.hh"
#include "common.hh"
#include "cursesui.hh"
#include "cursesui_pimpl.hh"

/****************************************************************************/
/** @name Display
 ** @{
 **/

/** Remove unprintable characters from strings (for showing them on screen) */
static Glib::ustring curses_escape(Glib::ustring const& str)
{
	Glib::ustring s;

	Glib::ustring::const_iterator i;
	for (i = str.begin();
	     i != static_cast<Glib::ustring::const_iterator>(str.end());
	     ++i) {
		if (Glib::Unicode::isprint(*i))
			s += *i;
		else
			s += ' ';
	}

	return s;
}

/// Calculate the optimal number for context lines
int CursesInterface::Pimpl::get_context_line_count()
{
	if (parent_.options().context_lines_ >= 0) {
		return parent_.options().context_lines_;
	} else {
		return LINES / 4;
	}
}

/**
 * Resize windows to right sizes. Layout is simply:
 * Word | File
 * Context
 * Minimenu
 * Input
 */
void CursesInterface::Pimpl::resize()
{
	refresh();

	mvwin(word_w_, 0, 0);
	wresize(word_w_, 1, COLS/2);

	mvwin(file_w_, 0, COLS/2);
	wresize(file_w_, 1, COLS/2);
	
	mvwin(context_w_, 2, 0);
	wresize(context_w_, LINES - 4, COLS);

	mvwin(minimenu_w_, LINES-2, 0);
	wresize(minimenu_w_, 1, COLS);

	mvwin(input_w_, LINES-1, 0);
	wresize(input_w_, 1, COLS);

	// Handle resizing context area, if automatic
	if (context_)
		context_->set_context_line_count(get_context_line_count());
	
	// Redraw
	redraw();
}

/**
 * Put the name of the current file in file window.
 */
void CursesInterface::Pimpl::redraw_file()
{
	werase(file_w_);
	wmove(file_w_, 0, 0);
	wprintw(file_w_, _("File: %s"), file_.c_str());
	wrefresh(file_w_);
}

/**
 * Redraw context window. Put all stuff in context in it, and
 * highlight the current word on current context line.
 * Also print available suggestions, if any, and number them.
 */
void CursesInterface::Pimpl::redraw_context()
{
	werase(context_w_);
	wmove(context_w_, 0, 0);

	if (context_) {
		Context::const_iterator i;
		for (i = context_->begin(); i != context_->end(); ++i) {
			if (i == context_->current()) {
				Glib::ustring p1(i->begin(),
						 context_->word_begin());
				Glib::ustring p2(context_->word());
				Glib::ustring p3(context_->word_end(),
						 i->end());
				
				wprintw(context_w_, "%s",
					parent_.to_locale(curses_escape(p1)).c_str());
				wattron(context_w_, A_STANDOUT);
				wprintw(context_w_, "%s",
					parent_.to_locale(curses_escape(p2)).c_str());
				wattroff(context_w_, A_STANDOUT);
				wprintw(context_w_, "%s\n",
					parent_.to_locale(curses_escape(p3)).c_str());
				
			} else {
				wprintw(context_w_, "%s\n",
					parent_.to_locale(curses_escape(*i)).c_str());
			}
		}
	}

	if (!suggestions_.empty()) {
		wprintw(context_w_, "\n");
		
		int idx = 0;
		std::vector<Glib::ustring>::const_iterator j;
		for (j = suggestions_.begin();
		     j != suggestions_.end();
		     ++j, ++idx)
		{
			if (idx <= 9) {
				wprintw(context_w_, " %d: ", idx);
			} else {
				wprintw(context_w_, " *: ");
			}
			wprintw(context_w_, "%s\n",
				parent_.to_locale(*j).c_str());
		}
	}
	
	wrefresh(context_w_);
}

/**
 * Redraw word window. Print the word context is currently at to it.
 */
void CursesInterface::Pimpl::redraw_word()
{
	werase(word_w_);
	if (context_) 
		wprintw(word_w_, "%s",
			parent_.to_locale(curses_escape(context_->word())).c_str());
	wrefresh(word_w_);
}

/**
 * Redraw minimenu.
 */
void CursesInterface::Pimpl::redraw_minimenu()
{
	werase(minimenu_w_);
	wmove(minimenu_w_, 0, 0);
	wprintw(minimenu_w_,
		parent_.to_locale(_("[SP] <number> R)epl A)ccept I)nsert L)ookup U)ncap Q)uit "
				    "e(X)it or ? for help")).c_str());
	wrefresh(minimenu_w_);
}

/**
 * Redraw all windows.
 * FIXME: This should maybe redraw also input_w_...
 */
void CursesInterface::Pimpl::redraw()
{
	redraw_word();	
	redraw_file();
	redraw_context();
	redraw_minimenu();
	
	werase(input_w_);
	wrefresh(input_w_);
}

/**
 * Show help text and prompt user for a key press.
 */
void CursesInterface::Pimpl::show_help()
{
	werase(context_w_);
	wprintw(context_w_, "%s",
		parent_.to_locale(_(""
		  "Whenever an unrecognized word is found, it is printed on\n"
		  "a line on the screen. If there are suggested corrections\n"
		  "they are listed with a number next to each one. You have\n"
		  "the option of replacing the word completely, or choosing\n"
		  "one of the suggested words. Alternatively, you can ignore\n"
		  "this word, ignore all its occurrences or add it in the\n"
		  "personal dictionary.\n"
		  "\n"
		  "Commands are:\n"
		  " r       Replace the misspelled word completely.\n"
		  " space   Accept the word this time only.\n"
		  " a       Accept the word for the rest of this session.\n"
		  " i       Accept the word, and put it in your personal dictionary.\n"
		  " u       Accept and add lowercase version to personal dictionary.\n"
		  " 0-9     Replace with one of the suggested words.\n"
		  " x       Write the rest of this file, ignoring misspellings,\n"
		  "         and start next file.\n"
		  " q       Quit immediately.  Asks for confirmation.\n"
		  "         Leaves file unchanged.\n"
		  " ^Z      Suspend program.\n"
		  " ?       Show this help screen.\n")).c_str());
	wrefresh(context_w_);
}

/**
 * Prompt the user to enter some text in input window.
 * @param prompt  Text to display the user.
 * @param maxlen  Maximum length of input. Note that maxlen==1 makes this to
 *                use getch.
 */
std::string CursesInterface::Pimpl::prompt(std::string const& prompt,
					   int maxlen)
{
	std::string str;

	werase(input_w_);
	wprintw(input_w_, "%s", parent_.to_locale(prompt).c_str());
	wrefresh(input_w_);

	if (maxlen == 1) {
		str += wgetch(input_w_);
	} else {
		char buf[maxlen+1];
		echo();
		wgetnstr(input_w_, buf, maxlen);
		noecho();
		str = buf;
	}

	werase(input_w_);
	wrefresh(input_w_);
	
	return str;
}

/** @} */


/*****************************************************************************/
/** @name Behaviour
 ** @{
 **/

/** Yes or no keys */
static char const* keys_yes_no = N_("yn");

/** Control keys: Add Insert Uncap Quit eXit Replace */
static char const* keys_control = N_("aiuqxr");

#define CKEY_YES     keys_yes_no[0]
#define CKEY_NO      keys_yes_no[1]

#define CKEY_SKIP    ' '
#define CKEY_ADD     keys_control[0]
#define CKEY_INSERT  keys_control[1]
#define CKEY_UNCAP   keys_control[2]
#define CKEY_QUIT    keys_control[3]
#define CKEY_EXIT    keys_control[4]
#define CKEY_REPLACE keys_control[5]
#define CKEY_HELP    '?'

#define IS_KEY(k, w) (tolower(k) == tolower(w))

/**
 * The exception to throw when user requests quitting
 */
class QuitException
{
public:
	QuitException() {}
};

/**
 * Ask for user what to do with a misspelled word, and perform
 * the action given.
 *
 * @throws QuitException When user wants to quit without saving.
 */
bool CursesInterface::Pimpl::handle_misspelled()
{
	suggestions_.clear();
	parent_.get_suggestions(context_->word(), suggestions_);

	redraw_word();
	redraw_context();
	redraw_minimenu();

	Glib::ustring word(context_->word());

	while (1) {
		int key = getch();

		// Skip this once
		if (IS_KEY(key, CKEY_SKIP)) {
			return true;
		}
		// Add to session dictionary
		else if (IS_KEY(key, CKEY_ADD)) {
			parent_.add_session_word(word);
			return true;
		}
		// Add to personal dictionary
		else if (IS_KEY(key, CKEY_INSERT)) {
			parent_.add_personal_word(word);
			return true;
		}
		// Add to personal dictionary, in lowercase
		else if (IS_KEY(key, CKEY_UNCAP)) {
			tolower(word);
			parent_.add_personal_word(word);
			return true;
		}
		// Quit wo/ saving
		else if (IS_KEY(key, CKEY_QUIT)) {
			if (dirty_) {
				std::string answer = prompt(
					_("Are you sure you want to "
					  "throw away your changes? (y/n): "),
					1);
				if (!IS_KEY(answer[0], CKEY_YES))
					continue;
			}
			throw QuitException();
		}
		// Next file with saving
		else if (IS_KEY(key, CKEY_EXIT)) {
			return false;
		}
		// Replace
		else if (IS_KEY(key, CKEY_REPLACE)) {
			std::string rep = prompt(_("Replace with: "), 512);
			context_->replace_word(parent_.from_locale(rep));
			dirty_ = true;
			return true;
		}
		// Show help
		else if (IS_KEY(key, CKEY_HELP)) {
			show_help();
			prompt(_("-- Press any key to continue --"), 1);
		}
		// Replace with a suggestion
		else if (key >= '0' && key <= '9') {
			int n = key - '0';
			if (n >= (signed)suggestions_.size()) {
				; // Do nothing
			} else {
				context_->replace_word(suggestions_[n]);
				dirty_ = true;
				return true;
			}
		}
		
		redraw_context();
	}

	return false;
}

/**
 * Check the spelling of a given file with given filter and replace
 * it with corrected version, if QuitException does not happen.
 * Makes backup copies if so wanted in options.
 */
void CursesInterface::Pimpl::check_file(std::string const& file,
					Options::FilterType type)
{
	// Open input file
	std::ifstream in(file.c_str());
	if (!in) throw Error(_("Unable to open file %s"), file.c_str());

	// Open output file
	// FIXME: Here I happily use stdio and iostream mixed...
	FILE* out;
	{
		out = tmpfile();
		if (!out) {
			throw Error(_("Unable to open temporary file"));
		}
	}

	// Display that we are editing a new file: indicate it
	dirty_ = false;
	file_ = file;
	redraw_file();

	/*
	 * Spell check:
	 */

	// Create suitable filter
	delete filter_;
	filter_ = parent_.create_filter(type);

	context_ = new Context(filter_, get_context_line_count(),
			       in, out, parent_);

	// Read lines
	while (context_->next_word())
	{
		if (!parent_.check_word(context_->word())) {
			if (!handle_misspelled()) {
				context_->flush();
				break;
			}
		}
	}

	delete context_;
	context_ = 0;

	in.close();

	// If the file was changed, save changes
	if (dirty_) {
		// Make backup if wanted
		if (parent_.options().backups_) {
			std::string backup = file + ".bak";
			rename(file.c_str(), backup.c_str());
		}
		
		// Then replace the original
		rewind(out);
		std::ofstream newout(file.c_str());
		char buf[1024];
		size_t readen;
		while ((readen = fread(buf, 1, 1024, out)) > 0) {
			newout.write(buf, readen);
		}
	}
	
	// Cleanup
	fclose(out);
}

/** @} */


/*****************************************************************************/
/** @name SIGWINCH (resize) handling
 ** @{
 **/

/**
 * The interface to resize on SIGWINCH
 */
CursesInterface::Pimpl* CursesInterface::Pimpl::interface_to_resize = 0;

/**
 * Handle resizing window
 */
static void resize_signal_handler(int sig)
{
	sig=sig;
	endwin();
	if (CursesInterface::Pimpl::interface_to_resize) {
		CursesInterface::Pimpl::interface_to_resize->resize();
	}
}

/** @} */


/*****************************************************************************/
/** @name Init and cleanup the private interface
 ** @{
 **/

/**
 * Clean up and end curses
 */
static void curses_cleanup()
{
        clear();
        refresh();
        resetty();
        endwin();
}

/**
 * Set up curses windows (and call ::redraw to place them). Setup signal
 * handling. Start spell checking files given in options.
 */
void CursesInterface::Pimpl::start()
{
	// Init curses
	initscr();
	savetty();
	cbreak();
	noecho();

	// Init windows
	word_w_  = newwin(1, 1, 0, 0);
	file_w_  = newwin(1, 1, 0, COLS/2);
	context_w_  = newwin(1, 1, 2, 0);
	minimenu_w_ = newwin(1, 1, 3, 0);
	input_w_    = newwin(1, 1, 4, 0);
	resize(); // I don't want to write resize code twice...

	// Init some translations
	keys_yes_no = _(keys_yes_no);
	keys_control = _(keys_control);

	// Handle changing terminal size
	interface_to_resize = this;
	signal(SIGWINCH, resize_signal_handler);

	redraw();

	// Check each file until quit requested
	std::vector< std::pair<std::string, Options::FilterType> >::const_iterator f;
	for (f = parent_.options().files_.begin();
	     f != parent_.options().files_.end();
	     ++f)
	{
		try {
			check_file(f->first, f->second);
		} catch (QuitException const& exc) {
			break;
		} catch (Error const& err) {
			curses_cleanup();
			throw;
		} 
	}
}

/**
 * Just init data.
 */
CursesInterface::Pimpl::Pimpl(IspellAlike& parent)
	: parent_(parent), file_(), context_(0), filter_(0), dirty_(false),
	  word_w_(0), file_w_(0), context_w_(0), minimenu_w_(0), input_w_(0)
{
}

/**
 * Cleanup. Terminate curses and delete dynamic objects.
 */
CursesInterface::Pimpl::~Pimpl()
{
	delete filter_;
	curses_cleanup();
}

/** @} */


/*****************************************************************************/
/** @name Context
 **
 ** FIXME: Will happily clobber CRLF to LF due to usage of std::getline.
 ** FIXME: This implementation is a bit too complex.
 ** @{
 **/

/**
 * Construct a new text context line buffer.
 * @param filter The filter to extract words with.
 * @param extra_lines How many lines of context to store.
 * @param in The input stream.
 * @param out The output stream.
 */
Context::Context(Filter* filter, int extra_lines, std::istream& in, FILE* out,
		 IspellAlike& parent)
	: parent_(parent), in_(in), out_(out), filter_(filter),
	  nlines_(1), current_pos_(0), current_(end()),
	  word_begin_(), word_end_()
{
	if (extra_lines > 0) nlines_ += extra_lines;

	fill_buffer();

	current_ = begin();
	current_pos_ = 0;
	if (current_ != end()) {
		filter_->set_line(&(*current_));
		filter_->reset();
	}
}

/**
 * Fill buffer with lines read from input until it is full.
 */
void Context::fill_buffer()
{
	std::string line;
	while ((signed)size() < nlines_ && std::getline(in_, line)) {
		push_back(parent_.from_user(line));
	}
}

/**
 * Write the first line in buffer to output, and remove it from buffer.
 * @return Whether something was written.
 */
bool Context::flush_first()
{
	if (!empty()) {
		fputs(parent_.to_user(front()).c_str(), out_);
		fputc('\n', out_);
		pop_front();
		return true;
	} else {
		return false;
	}
}

/**
 * Write whole buffer to output. Also read all available data from input
 * and write it to output as-is.
 */
void Context::flush()
{
	char buf[1024];
	size_t readen;
	while (flush_first());
	while ((readen = in_.rdbuf()->sgetn(buf, 1024)) > 0) {
		fwrite(buf, 1, readen, out_);
	}
}

/**
 * Adjust the number of extra context lines shown.
 */
void Context::set_context_line_count(int extra_lines)
{
	nlines_ = 1;
	if (extra_lines > 0) nlines_ += extra_lines;

	// Flush excessive context then...
	while ((signed)size() > nlines_ && current_ != begin()) {
		flush_first();
		--current_pos_;
	}
}

/**
 * Advance to the next word with the current filter. Read new lines from
 * input, if necessary.
 * @return Whether there was a next word.
 */
bool Context::next_word()
{
	while (!empty())
	{
		if (filter_->get_next_word(&word_begin_, &word_end_)) {
			return true;
		} else {
			if (current_pos_ >= (nlines_+1)/2-1) {
				if (flush_first()) --current_pos_;
				fill_buffer();
			}
			if (current_pos_ < 0) {
				current_pos_ = 0;
				current_ = begin();
			} else {
				++current_pos_;
				++current_;
			}
			if (current_ == end()) return false;
			
			filter_->set_line(&(*current_));
		}
	}
	return false;
}

/**
 * Replace the current word with given string.
 */
void Context::replace_word(Glib::ustring const& replacement)
{
	int pos = std::distance(
		static_cast<Glib::ustring::const_iterator>(current_->begin()),
		word_begin_);
	current_->replace(pos, std::distance(word_begin_, word_end_),
			  replacement);

	Glib::ustring::const_iterator i = current_->begin();
	std::advance(i, pos);
	filter_->reset(i);
}

/**
 * Destroy a context.
 */
Context::~Context()
{
	flush();
}

/** @} */


/*****************************************************************************/
/** @name Public interface
 ** @{
 **/

/** Init implementation */
CursesInterface::CursesInterface(IspellAlike& parent)
{
	pimpl_ = new CursesInterface::Pimpl(parent);
}

/** Destroy implementation */
CursesInterface::~CursesInterface()
{
	delete pimpl_;
}

/** Start spell checker interface */
void CursesInterface::start()
{
	pimpl_->start();
}

/** @} */
