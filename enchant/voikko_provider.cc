/*-*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* enchant-voikko
 * Copyright (C) 2004  Pauli Virtanen
 *               2006  Harri Pitkänen <hatapitk@iki.fi>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>

#include <enchant.h>
#include <enchant-provider.h>

#include <glib.h>

#include "config.hh"
#include "spell.hh"
#include "config_file.hh"
#include "tmerror.hh"

#include "glibmm/ustring.h"

ENCHANT_PLUGIN_DECLARE("Voikko")

/*****************************************************************************/
/** Helper functions
 ** @{ */

/// Test whether a file exists
static std::string exstf(std::string const& fn)
{
	if (::g_file_test(fn.c_str(), ::G_FILE_TEST_EXISTS))
		return fn;
	else
		return std::string();
}

/**
 * Resolve the library and dictionary files to use, for Finnish only.
 * 1. First try Enchant registry.
 * 2. Then try Tmispell config file.
 */
static void
voikko_checker_get_files(std::string& library, std::string& dictionary)
{
	char *lib = NULL;
	char *dict = NULL;

	library.clear();
	dictionary.clear();

	/* Try to get from Enchant registry */
	
	lib = ::enchant_get_registry_value("Voikko", "Library");
	if (lib) library = exstf(lib);
	::g_free(lib);
	
	dict = ::enchant_get_registry_value("Voikko", "Dictionary");
	if (dict) dictionary = exstf(dict);
	::g_free(dict);

	/* Try to get from Tmispell config files */

	static char* cfgfiles[] = {
		CONFIG_FILE,
		NULL
	};
	
	static char* langnames[] = {
		"suomi",
		"finnish",
		NULL
	};
	
	int i, j;
	for (i = 0; cfgfiles[i] != NULL; ++i) {
		if (!dictionary.empty() && !library.empty()) break;
		try {
			ConfigFile conf(cfgfiles[i]);
			for (j = 0; langnames[j] != NULL; ++j) {
				char* l = langnames[j];
				if (!conf.has(l)) continue;
				const SpellcheckerEntry &e = conf.get(l);
				
				if (library.empty())
					library = exstf(e.get_library());
				if (dictionary.empty())
					dictionary = exstf(e.get_dictionary());
			}
		} catch (...) {
			/* noop: fail silently */
		}
	}
}

/**
 * Check the spelling of the given word.
 */
static int
voikko_dict_check (EnchantDict * me, const char *const word, size_t len)
{
	Spellchecker *manager;
	bool ok;

	if (word == NULL || len == 0) return 0;

	manager = reinterpret_cast<Spellchecker *>(me->user_data);

	ok = manager->check_word(word);
	
	return ok ? 0 : 1;
}

/**
 * Generate suggestions for the given word.
 */
static char **
voikko_dict_suggest (EnchantDict * me, const char *const word,
		     size_t len, size_t * out_n_suggs)
{
	Spellchecker *manager;
	std::vector<Glib::ustring> suggestions;

	char **sugg_arr = NULL;

	if (word == NULL || len == 0) return NULL;
	
	manager = reinterpret_cast<Spellchecker *>(me->user_data);
	manager->get_suggestions(word, suggestions);

	*out_n_suggs = suggestions.size();
	
	if (!suggestions.empty())
	{
		sugg_arr = g_new0(char*, suggestions.size() + 1);

		unsigned int i;
		for (i = 0; i < suggestions.size(); ++i) {
			sugg_arr[i] = strdup(suggestions[i].c_str());
		}
	}
	return sugg_arr;
}

/**
 * Initialize a spellchecker instance.
 */
static Spellchecker *
voikko_request_manager(struct str_enchant_provider * me)
{
	Spellchecker * manager = NULL;
	try {
		std::string lib, dict;
		voikko_checker_get_files(lib, dict);
#ifdef DEBUG
		fprintf(stderr, "Creating manager %s %s\n", lib.c_str(),
			dict.c_str());
#endif
		/* Enchant always uses utf-8 encoding internally */
		manager = new Spellchecker(lib, dict, "utf-8");
	} catch (Error const& err) {
		enchant_provider_set_error(me, err.what());
		manager = NULL;
	}

	return manager;
}

/** @} */


/*****************************************************************************/
/** Provider callback functions
 ** @{ */

static int
voikko_provider_dictionary_exists (struct str_enchant_provider * me, 
				   const char *const tag)
{
#ifdef DEBUG
	fprintf(stderr, "Checking for tag %s\n", tag);
#endif
	if (strncmp(tag, "fi", 2) != 0) return 0;
	if (strlen(tag) == 2 || tag[2] == '_') return 1;
	return 0;
}

/// Free a strv list
static void
voikko_provider_free_string_list (EnchantProvider * me, char **str_list)
{
	::g_strfreev(str_list);
}

static EnchantDict *
voikko_provider_request_dict (EnchantProvider * me, const char *const tag)
{
	EnchantDict *dict = NULL;
	Spellchecker *manager = NULL;

#ifdef DEBUG
	fprintf(stderr, "Asking for tag %s\n", tag);
#endif
	
	if (!voikko_provider_dictionary_exists(me, tag))
		return NULL;
	
	manager = voikko_request_manager(me);

	if (!manager) 
		return NULL;

	dict = g_new0(EnchantDict, 1);
	dict->user_data = manager;
	dict->check = voikko_dict_check;
	dict->suggest = voikko_dict_suggest;
	/* don't use personal, session - let higher level implement that */
	
	return dict;
}

static void
voikko_provider_dispose_dict (EnchantProvider * me, EnchantDict * dict)
{
	Spellchecker *manager
		= reinterpret_cast<Spellchecker *>(dict->user_data);
	delete manager;
	::g_free (dict);
}

static void
voikko_provider_dispose (EnchantProvider * me)
{
	::g_free (me);
}

static char *
voikko_provider_identify (EnchantProvider * me)
{
	return "voikko";
}

static char *
voikko_provider_describe (EnchantProvider * me)
{
	return "Voikko Provider";
}

extern "C" {
	ENCHANT_MODULE_EXPORT (EnchantProvider *) 
	init_enchant_provider (void)
	{
		EnchantProvider *provider;
		
		provider = g_new0 (EnchantProvider, 1);
		provider->dispose = voikko_provider_dispose;
		provider->request_dict = voikko_provider_request_dict;
		provider->dispose_dict = voikko_provider_dispose_dict;
		provider->dictionary_exists = voikko_provider_dictionary_exists;
		provider->identify = voikko_provider_identify;
		provider->describe = voikko_provider_describe;
		provider->free_string_list = voikko_provider_free_string_list;
		
		return provider;
	}
}

/** @} */
