/**
 * @file common.hh
 * @author Pauli Virtanen
 *
 * Common utility functions.
 */
#ifndef COMMON_HH_
#define COMMON_HH_

#include <string>

#include "glibmm/ustring.h"

/// Sprintf to a std::string.
std::string ssprintf(char const* fmt, ...);

/// Convert the given string to lowercase in-place.
void tolower(Glib::ustring& str);

/// Convert the given string to uppercase in-place.
void toupper(Glib::ustring& str);

#endif
