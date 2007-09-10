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
 * @file i18n.cc
 * @author Pauli Virtanen
 *
 * Localization etc.
 */
#include <locale.h>

#include "config.hh"
#include "i18n.hh"

#ifdef ENABLE_NLS

#  include <libintl.h>

const char* _(char const* str)
{
	if (str && str[0] != '\0')
		return dgettext(PACKAGE, str);
	else
		return "";
}

std::string _(std::string const& str)
{
	if (!str.empty())
		return std::string(dgettext(PACKAGE, str.c_str()));
	else
		return std::string();
}

void locale_init()
{
        setlocale(LC_ALL, "");

	bindtextdomain(PACKAGE, PACKAGE_LOCALE_DIR);
	textdomain(PACKAGE);
}

#else // ENABLE_NLS

void locale_init()
{
}

#endif
