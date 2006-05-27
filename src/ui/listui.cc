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
