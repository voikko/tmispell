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
