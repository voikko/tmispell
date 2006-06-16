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
