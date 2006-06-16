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
 * @file cursesui.hh
 * @author Pauli Virtanen
 *
 * An ispell-like curses text interface.
 */
#ifndef CURSESUI_HH_
#define CURSESUI_HH_

class IspellAlike;

/**
 * A text-mode user interface.
 */
class CursesInterface
{
public:
	CursesInterface(IspellAlike& parent);
	~CursesInterface();
	
	void start();

public:
	class Pimpl;
	friend class Pimpl;

private:
	Pimpl* pimpl_;
};

#endif // CURSESUI_HH_
