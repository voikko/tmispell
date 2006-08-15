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

#include <enchant.h>
#include <enchant-provider.h>

#include <glib.h>

#include <libvoikko/voikko.h>

ENCHANT_PLUGIN_DECLARE("Voikko")

int voikko_handle;

/**
 * Check the spelling of the given word.
 */
static int
voikko_dict_check (EnchantDict * me, const char *const word, size_t len)
{
	bool ok;

	if (word == NULL || len == 0) return 0;

	ok = voikko_spell_cstr(voikko_handle, word);
	
	return ok ? 0 : 1;
}

/**
 * Generate suggestions for the given word.
 */
static char **
voikko_dict_suggest (EnchantDict * me, const char *const word,
		     size_t len, size_t * out_n_suggs)
{
	char **sugg_arr;
	if (word == NULL || len == 0) return NULL;
	sugg_arr = voikko_suggest_cstr(voikko_handle, word);
	if (sugg_arr == NULL) return NULL;
	for (*out_n_suggs = 0; sugg_arr[*out_n_suggs] != NULL; (*out_n_suggs)++);
	return sugg_arr;
}


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
	const char * voikko_error;
	
	if (!voikko_provider_dictionary_exists(me, tag))
		return NULL;

	voikko_error = voikko_init(&voikko_handle, "fi_FI", 0);
	if (voikko_error) return NULL;

	dict = g_new0(EnchantDict, 1);
	dict->check = voikko_dict_check;
	dict->suggest = voikko_dict_suggest;
	/* don't use personal, session - let higher level implement that */
	
	return dict;
}

static void
voikko_provider_dispose_dict (EnchantProvider * me, EnchantDict * dict)
{
	voikko_terminate(voikko_handle);
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
#ifdef HAVE_ENCHANT_1_1_6
		provider->free_string_list = voikko_provider_free_string_list;
#endif
		
		return provider;
	}
}

