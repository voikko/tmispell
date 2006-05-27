/**
 * @file filter.hh
 * @author Pauli Virtanen
 *
 * Extracting the parts from input that require spell checking.
 */
#ifndef FILTER_HH_
#define FILTER_HH_

#include "glibmm/ustring.h"

#include "options.hh"

/**
 * An interface to retrieve words from a text stream.
 */
class Filter
{
public:
	virtual ~Filter() {}
	
	/// Return a filter of a given type
	static Filter* new_filter(Options::FilterType type,
				  Options const& options);

	/// Reset position to the beginning of the line
	virtual void reset(Glib::ustring::const_iterator pos) { pos=pos; }
	virtual void reset() { reset(line_->begin()); }

	/// Set the line to be filtered
	virtual void set_line(Glib::ustring const* line) { line_ = line; }

	/// Get the line to be filtered
	virtual Glib::ustring const& get_line() const { return *line_; }

	/// Get the next whole word in line
	virtual bool get_next_word(Glib::ustring::const_iterator* found_begin,
				   Glib::ustring::const_iterator* found_end)=0;

protected:
	/// The line to be filtered
	Glib::ustring const* line_;
};

#endif // FILTER_HH_
