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
