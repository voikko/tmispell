#ifndef TMERROR_HH_
#define TMERROR_HH_

#include <stdexcept>
#include <string>

/**
 * A generic base class for run time errors with unlimited message length.
 */
class Error : public std::runtime_error
{
public:
	/// Throw an error with printfed error message
	explicit Error(char* const fmt, ...);
	explicit Error(std::string const& msg);
	virtual ~Error() throw() {}
	/// Return an error description
	virtual const char* what() const throw() { return msg_.c_str(); }

protected:
	/// The string describing the error
	std::string msg_;
};

#endif
