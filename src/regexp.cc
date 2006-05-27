#include <stdlib.h>
#include <regex.h>

#include <string>

#include "regexp.hh"
#include "tmerror.hh"

/** Returns an error message corresponding to the given regcomp error code */
static Error get_regerror(int errcode, regex_t const* compiled)
{
	unsigned long length = regerror(errcode, compiled, NULL, 0);
	char buffer[length];
	regerror(errcode, compiled, buffer, length);
	return Error(buffer);
}

/** Compiles the given pattern to a regular expression */
RegExp::RegExp(const char* regstr, Flag flags)
	: matches_(0)
{
	int regex_flags =
		((flags & EXTENDED) ? REG_EXTENDED : 0) | 
		((flags & ICASE) ? REG_ICASE : 0) |
		((flags & NOSUB) ? REG_NOSUB : 0) |
		((flags & NEWLINE) ? REG_NEWLINE : 0);

	int error_code = regcomp(&regexp_, regstr, regex_flags);

	if (error_code != 0) {
		throw get_regerror(error_code, &regexp_);
	}

	matches_ = new regmatch_t[regexp_.re_nsub + 1];
}

/** Destroys this regular expression */
RegExp::~RegExp()
{
	regfree(&regexp_);
	delete [] matches_;
	matches_ = 0;
}

/** Attempts to match this regular expression to a string */
bool RegExp::do_match(const char* str)
{
	int code = regexec(&regexp_, str, regexp_.re_nsub + 1, matches_, 0);

	if (code == 0)
		return true;
	else if (code == REG_NOMATCH)
		return false;
	else
		throw get_regerror(code, &regexp_);
}


/** 
 * Returns the begin position of the nth subexpression in the last
 * successfully matched string.
 */
unsigned int RegExp::begin(unsigned int n) const
{
	if (n <= regexp_.re_nsub && matches_[n].rm_so >= 0) {
		return matches_[n].rm_so;
	} else {
		return 0;
	}
}

/** 
 * Returns the position after the end of the nth subexpression in the last
 * successfully matched string.
 */
unsigned int RegExp::end(unsigned int n) const
{
	if (n <= regexp_.re_nsub && matches_[n].rm_eo >= 0) {
		return matches_[n].rm_eo;
	} else {
		return 0;
	}
}
