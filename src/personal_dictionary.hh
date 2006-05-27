/**
 * @file personal_dictionary.hh
 * @author Pauli Virtanen
 *
 * A personal dictionary for the user.
 */
#ifndef PERSONAL_DICTIONARY_HH_
#define PERSONAL_DICTIONARY_HH_

#include <set>
#include <string>

#include "glibmm/ustring.h"

/**
 * A word associated with its capitalization status
 */
class CapitalizedWord
{
public:
	/// A capitalization status.
	typedef enum { lower, upper, first, other } Capitalization;

public:
	/// Construct a new word and scan its capitalization
	CapitalizedWord(Glib::ustring const& word);

	/// Get this word properly capitalized
	Glib::ustring get_word() const;

	/// Get the capitalization type of this word
	Capitalization get_capitalization() const
		{ return capitalization_; }

	/// Compare words without capitalization
	bool operator<(CapitalizedWord const& a) const
		{ return word_ < a.word_; }

private:
	/// The capitalization status of this word
	Capitalization capitalization_;

	/// The word in lower case
	Glib::ustring word_;
};

/**
 * A set of words that can be saved and loaded from a file.
 */
class PersonalDictionary
{
public:
	/// Construct a new empty personal dictionary
	PersonalDictionary() : words_(), changed_(false) {}

	/// Return true if the personal dictionary is changed since last save
	bool is_changed() const { return changed_; }

	/// Add words from the given file to this dictionary
	void merge(std::string const& filename);

	/// Save words in this dictionary to a file
	void save(std::string const& filename);

	/// Load the words for this dictionary from the given file
	void load(std::string const& filename)
		{ words_.clear(); merge(filename); changed_ = false; }

	/// Add the given word to this dictionary
	void add_word(Glib::ustring const& word)
		{ words_.insert(CapitalizedWord(word)); changed_ = true; }

	/// Check, if the given word is in this dictionary
	bool check_word(Glib::ustring const& word) const;

	/// Remove a word from this dictionary
	void remove_word(Glib::ustring const& word)
		{ words_.erase(CapitalizedWord(word)); changed_ = true; }

private:
	/// Type for a set of words
	typedef std::set<CapitalizedWord> WordSet;

	/// The set of words in this dictionary
	WordSet words_;

	/// Has the personal dictionary been changed
	bool changed_;
};

#endif // PERSONAL_DICTIONARY_HH_
