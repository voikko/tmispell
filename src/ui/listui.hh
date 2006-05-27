/**
 * @file listui.hh
 * @author Pauli Virtanen
 *
 * An interface that just lists misspelled words.
 */
#ifndef LISTUI_HH_
#define LISTUI_HH_

#include "tmispell.hh"

/**
 * An interface that just lists misspelled words.
 */
class ListInterface
{
public:
	ListInterface(IspellAlike& parent) : parent_(parent) {}
	void start();
private:
	IspellAlike& parent_;
};

#endif // LISTUI_HH_
