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
 * @file pipeui.hh
 * @author Pauli Virtanen
 *
 * An ispell -a pipe interface.
 */
#ifndef PIPEUI_HH_
#define PIPEUI_HH_

#include "tmispell.hh"
#include "glibmm/ustring.h"

/**
 * An interface that reads commands from stdin and acts according
 * to them.
 */
class PipeInterface
{
public:
	PipeInterface(IspellAlike& parent) : parent_(parent) {}
	
	void start();
	
private:
	/** Start listening for commands */
	void start_pipe_listen();
	
	/** Listen for more commands */
	void listen_pipe(std::istream& in);
	
	/** Interpret a command line */
	void interpret_pipe_command(Glib::ustring str);
	
	/** Spell check a word and output response */
	void spell_check_pipe(Glib::ustring const& str,
			      Glib::ustring::const_iterator str_begin);
	void spell_check_pipe(Glib::ustring const& str)
		{ spell_check_pipe(str, str.begin()); }
	
private:
	IspellAlike& parent_;
	
	/** The currently active filter */
	Filter* filter_;
	
	/** Current include depth */
	long include_depth_;
	
	/** Is this interface in terse output mode */
	bool terse_;
};

#endif // PIPEUI_HH_
