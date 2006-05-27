/**
 * @file i18n.hh
 * @author Pauli Virtanen
 *
 * Localization etc.
 */
#ifndef I18N_HH_
#define I18N_HH_

#include <string>

#include "config.hh"

#ifdef ENABLE_NLS
char* _(char const* str);
std::string _(std::string const& str);
#else // ENABLE_NLS
#define _(str) (str)
#endif // ENABLE_NLS

#define N_(str) (str)

///
void locale_init();

#endif // I18N_HH_
