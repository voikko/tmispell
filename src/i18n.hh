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
 * @file i18n.hh
 * @author Pauli Virtanen
 *
 * Localization etc.
 */
#ifndef I18N_HH_
#define I18N_HH_

#include <string>

#include "config.hh"

#ifdef ENABLE_NLS
const char* _(char const* str);
std::string _(std::string const& str);
#else // ENABLE_NLS
#define _(str) (str)
#endif // ENABLE_NLS

#define N_(str) (str)

///
void locale_init();

#endif // I18N_HH_
