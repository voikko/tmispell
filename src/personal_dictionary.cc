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
 * @file personal_dictionary.cc
 * @author Pauli Virtanen
 *
 * A personal dictionary for the user. Saving and loading.
 */
#include <fstream>
#include <map>
#include <algorithm>
#include <string>

#include "i18n.hh"
#include "common.hh"
#include "personal_dictionary.hh"
#include "charset.hh"

#include "glibmm/ustring.h"
#include "glibmm/convert.h"

void PersonalDictionary::merge(std::string const& filename)
{
	std::ifstream in(filename.c_str());
	if (!in.is_open()) 
		throw Error(_("Unable to open file %s for "
			      "reading a dictionary."),
			    filename.c_str());

	CharsetConverter conv("UTF-8");
	
	std::string str;
	while (in >> str) {
		words_.insert(conv.from(str));
	}
}

void PersonalDictionary::save(std::string const& filename)
{
	std::ofstream out(filename.c_str());
	if (!out.is_open())
		throw Error(_("Unable to open file %s for "
			      "writing a dictionary."), filename.c_str());

	CharsetConverter conv("UTF-8");
	
	WordSet::const_iterator i;
	for (i = words_.begin(); i != words_.end(); ++i) {
		out << conv.to(i->get_word()) << std::endl;
	}

	changed_ = false;
}

/*
 * Handle word capitalization.
 *
 * The goal is the following:
 *     1. All-lowercase words match either first-letter-capitalized or fully
 *        capitalized word.
 *     2. Other capitalizations match only themselves.
 */

/** Return the type of the capitalization of a word */
static CapitalizedWord::Capitalization get_capitalization(
	Glib::ustring const& word)
{
	Glib::ustring::const_iterator p = word.begin();
	
	if (p == word.end()) return CapitalizedWord::other;
	
	if (Glib::Unicode::isupper(*p)) {
		++p;
		if (p == word.end()) return CapitalizedWord::upper;
		
		if (Glib::Unicode::islower(*p)) {
			do { ++p; } while (p != word.end()
					   && Glib::Unicode::islower(*p));
			return (p == word.end()) ? 
				CapitalizedWord::first :
				CapitalizedWord::other;
		} else if (Glib::Unicode::isupper(*p)) {
			do { ++p; } while (p != word.end()
					   && Glib::Unicode::isupper(*p));
			return (p == word.end()) ? 
				CapitalizedWord::upper : 
				CapitalizedWord::other;
		}
	} else {
		while (p != word.end()
		       && Glib::Unicode::islower(*p)) ++p;
		return (p == word.end()) ? 
			CapitalizedWord::lower : 
			CapitalizedWord::other;
	}
	return CapitalizedWord::other;
}

/** Check whether a given word is in the dictionary */
bool PersonalDictionary::check_word(Glib::ustring const& word) const
{
	CapitalizedWord w(word);
	std::set<CapitalizedWord>::const_iterator p;
	
	/** Find the first word that is equal as a string */
	p = std::lower_bound(words_.begin(), words_.end(), w);
	
	/** Find the first one that has also a matching capitalization */
	while (p != words_.end() && !(w < *p)) {

		if (p->get_capitalization() == w.get_capitalization() ||
		    (p->get_capitalization() == CapitalizedWord::lower &&
		     (w.get_capitalization() == CapitalizedWord::upper ||
		      w.get_capitalization() == CapitalizedWord::first))) {
			 return true;
		}
		++p;
	}
	return false;
}

/** Creates a new capitalized word object */
CapitalizedWord::CapitalizedWord(Glib::ustring const& word)
	: capitalization_(::get_capitalization(word)),
	  word_(word)
{
	if (capitalization_ != CapitalizedWord::other &&
	    capitalization_ != CapitalizedWord::lower)
		tolower(word_);
}

/** Returns a string representation of a capitalized word */
Glib::ustring CapitalizedWord::get_word() const
{
	switch (capitalization_) {
		/** These are stored as-is */
	case other:
	case lower:
		return word_;

		/** Convert to upper case */
	case upper:
	{
		Glib::ustring s(word_);
		toupper(s);
		return s;
	}

		/** Capitalize first letter */
	case first:
	{
		Glib::ustring s(word_);
		if (s.length() > 0) {
			s.replace(0, 1, 1, (gunichar)toupper(s[0]));
		}
		return s;
	}
	}

	// This should never happen, so no need for localization
	throw std::string("FIXME: Unknown capitalization type.");
	return Glib::ustring();
}

#if TEST

#include <iostream>

int main()
{
	try {
		PersonalDictionary foo;
		std::cout << "A" << std::endl;
		foo.load("test.dict");
		std::cout << "B" << std::endl;
		foo.save("quux.dict");
		std::cout << "C" << std::endl;
		
		std::string s;
		while (std::cin >> s) {
			if (foo.check_word(s)) {
				std::cerr << "OK" << std::endl;
			} else {
				std::cerr << "NOT OK" << std::endl;
			}
		}
	} catch (std::string const& s) {
		std::cerr << s << std::endl;
	}
}

#endif
