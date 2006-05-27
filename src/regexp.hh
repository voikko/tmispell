/**
 * @file regexp.hh
 * @author Pauli Virtanen
 *
 * POSIX regular expressions.
 */
#ifndef REGEXP_HH_
#define REGEXP_HH_

#include "glibmm/ustring.h"
#include "common.hh"
#include <string>
#include <regex.h>

/**
 * A POSIX regular expression.
 */
class RegExp
{
public:
	/// Flags that the regular expression may have
	typedef int Flag;

	/// The regular expression is an extended regexp
	static const Flag EXTENDED = 0x1;

	/// The regular expression ignores character case
	static const Flag ICASE = 0x2;

	/// The regular expression has no subexpressions
	static const Flag NOSUB = 0x4;

	/// The regular expression recognizes newlines
	static const Flag NEWLINE = 0x8;

public:
	/// Compiles a pattern to a regular expresion
	RegExp(const char* str, Flag flags = 0x0);

	/// Destroys a regular expression
	~RegExp();
	
	/// Matches a regular expression to a string
	bool match(const char* str)
		{ offset_ = 0; return do_match(str); }

	/// Matches a regular expression to a std::string
	bool match(std::string const& str)
		{ offset_ = 0; return do_match(str.c_str()); }

	/// Matches a regular expression to a std::string, with offset
	bool match(std::string const& str, std::string::const_iterator p)
		{
			offset_ = p - str.begin();
			return do_match(str.c_str() + offset_);
		}
	
	/// Matches a regular expression to a Glib::ustring
	bool match(Glib::ustring const& str)
		{ offset_ = 0; return do_match(str.c_str()); }

	/// Matches a regular expression to a Glib::ustring, with offset
	bool match(Glib::ustring const& str, Glib::ustring::const_iterator p)
		{
			offset_ = p.base() - str.begin().base();
			return do_match(str.c_str() + offset_);
		}
	
	/// The begin of #nth subexpression in last matched string (0=regexp)
	unsigned int begin(unsigned int nth = 0) const;

	/// The end of #nth subexpression in last matched string (0=regexp)
	unsigned int end(unsigned int nth = 0) const;

	/// Return the #nth mathed subexpression in the string
	std::string sub(const std::string& str, unsigned int nth = 0) const
		{
			return str.substr(begin(nth) + offset_,
					  end(nth) - begin(nth));
		}

	/// Return the #nth mathed subexpression in the Glib::ustring
	Glib::ustring sub(const Glib::ustring& str, unsigned int nth = 0) const
		{
			const char* data = str.data();
			return Glib::ustring(data + offset_ + begin(nth),
					     end(nth) - begin(nth));
		}

private:
	bool do_match(const char* str);
	
	/// The compiled POSIX regexp
	regex_t regexp_;

	/// Information about last matched subexpressions
	regmatch_t* matches_;

	// Extra offset
	int offset_;
};

#endif
