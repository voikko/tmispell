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
 * @file cursesui_pimpl.hh
 * @author Pauli Virtanen
 *
 * An ispell-like curses text interface. [private implementation]
 */
#ifndef CURSESUI_PIMPL_HH
#define CURSESUI_PIMPL_HH

#include <iostream>
#include <string>
#include <vector>
#include <list>

#include <stdio.h>

#include <curses.h>

#include "cursesui.hh"
#include "tmispell.hh"

/**
 * Storage of context text lines.
 */
class Context : public std::list<Glib::ustring>
{
public:
	/// Construct a context buffer
	Context(Filter* filter_, int extra_lines, std::istream& in, FILE* out,
		IspellAlike& parent);

	/// Destroy a context buffer
	~Context();

	/// Set the number of extra context lines to show
	void set_context_line_count(int extra_lines);

	/// Advance to the next word
	bool next_word();

	/// The current word
	Glib::ustring word() const {
		return Glib::ustring(word_begin_,word_end_);
	}

	/// Iterator to the current line
	iterator current() const { return current_; }

	/// Iterator to the begin of the current word
	Glib::ustring::const_iterator word_begin() const { return word_begin_;}

	/// Iterator to the end of the current word
	Glib::ustring::const_iterator word_end() const { return word_end_; }

	/// Replace the current word with the given string
	void replace_word(Glib::ustring const& replacement);

	/// Flush remaining data from buffers and input to output
	void flush();

private:
	/// Flush the first line in buffer to output
	bool flush_first();

	/// Fill the buffer with lines read from input
	void fill_buffer();

private:
	/// The spell checker engine to use
	IspellAlike& parent_;

	/// Input stream
	std::istream& in_;

	/// Output stream
	FILE* out_;

	/// Input filter
	Filter* filter_;
	
	/// Number of lines to hold
	int nlines_;
	
	/// The position of current line in fifo
	int current_pos_;
	
	/// The line to be spell checked
	iterator current_;

	/// The begin of misspelled word on current line
	Glib::ustring::const_iterator word_begin_;

	/// The end of misspelled word on current line
	Glib::ustring::const_iterator word_end_;
};

/**
 * A text-mode user interface.
 */
class CursesInterface::Pimpl
{
public:
	/// Init data structures
	Pimpl(IspellAlike& parent);

	/// Destroy and cleanup
	~Pimpl();

	/// Start the curses interface
	void start();

public:
	/// The interface to resize on SIGWINCH
	static Pimpl* interface_to_resize;
	
	/// Resize the UI to fit screen
	void resize();
	
private:
	/// Redraw all static windows.
	void redraw();

	/// Redraw the word window.
	void redraw_word();
	
	/// Redraw the file name window
	void redraw_file();

	/// Fill context area with context fifo w/highlight and suggestions.
	void redraw_context();

	/// Draw minimenu
	void redraw_minimenu();

	/// Show help text and wait for keypress
	void show_help();

	/// Check a file of given type
	void check_file(std::string const& file, Options::FilterType type);

	/// Calculate the optimal number for context lines
	int get_context_line_count();

	/// Ask user what to do with misspelled word and do it
	bool handle_misspelled();

	/// Prompt user for a string of given length or shorter.
	std::string prompt(std::string const& prompt, int maxlen);
	
private:
	/// The spell checker engine to use
	IspellAlike& parent_;

	/// The file that is currently being spell checked
	std::string file_;

	/// The lines of context
	Context* context_;

	/// Suggestions
	std::vector<Glib::ustring> suggestions_;

	/// The filter currently in use
	Filter* filter_;

	/// Has this file been modified?
	bool dirty_;

	/// A window for displaying a misspelled word
	WINDOW* word_w_;

	/// A window for displaying a file name
	WINDOW* file_w_;

	/// A window for displaying context and suggestions
	WINDOW* context_w_;

	/// A window for displaying minimenu
	WINDOW* minimenu_w_;

	/// A window for prompting user for input
	WINDOW* input_w_;
};

#endif // CURSESUI_PIMPL_HH
