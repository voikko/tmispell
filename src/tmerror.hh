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
