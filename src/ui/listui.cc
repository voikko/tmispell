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
 * @file listui.cc
 * @author Pauli Virtanen
 *
 * An interface that just lists misspelled words.
 */

#include <string>
#include <iostream>

#include "listui.hh"
#include "tmispell.hh"

#include "charset.hh"

#include "glibmm/ustring.h"

/**
 * Read words from stdin and print misspelled words to stdout.
 * They are always printed to stdout to be compatible w/ ispell.
 */
void ListInterface::start()
{
	Filter* filter = parent_.create_default_filter();

	std::string line;
	while (std::getline(std::cin, line))
	{
		Glib::ustring uline = parent_.from_user(line);
		filter->set_line(&uline);

		Glib::ustring::const_iterator begin, end;
		while (filter->get_next_word(&begin, &end))
		{
			Glib::ustring word(begin, end);
			if (!parent_.check_word(word)) {
				std::cout << parent_.to_user(word)
					  << std::endl;
			}
		}
	}

	delete filter;
}
