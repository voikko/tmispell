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

char* _(char const* str)
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
