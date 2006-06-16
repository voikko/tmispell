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
