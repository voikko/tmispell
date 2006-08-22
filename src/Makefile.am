INCLUDES=$(GLIB_CFLAGS) -Iglibmm $(CFLAGS)

USTRING_SOURCES = glibmm/convert.h glibmm/error.h glibmm/exception.h glibmm/unicode.h \
                  glibmm/ustring.h glibmm/utility.h glibmm/convert.cc glibmm/error.cc \
                  glibmm/exception.cc glibmm/glibmmconfig.h glibmm/unicode.cc glibmm/ustring.cc \
                  glibmm/private/convert_p.h glibmm/private/unicode_p.h


MAINTAINERCLEANFILES =  Makefile.in .deps config.hh config.hh.in stamp-h.in

bin_PROGRAMS = tmispell
tmispell_LDADD = @INTLLIBS@ $(GLIB_LIBS)
tmispell_LDFLAGS = -lvoikko
tmispell_SOURCES =	\
	charset.hh	\
	charset.cc	\
	common.hh	\
	common.cc	\
	config.hh	\
	config_file.cc	\
	config_file.hh	\
	personal_dictionary.cc	\
	personal_dictionary.hh	\
	filter.cc	\
	filter.hh	\
	i18n.hh		\
	i18n.cc		\
	options.cc	\
	options.hh	\
	ui/pipeui.cc	\
	ui/pipeui.hh	\
	ui/listui.cc	\
	ui/listui.hh	\
	ui/cursesui.cc	\
	ui/cursesui.hh	\
	ui/cursesui_pimpl.hh	\
	regexp.cc	\
	regexp.hh	\
	tmerror.cc	\
	tmerror.hh	\
	tmispell.cc	\
	tmispell.hh	\
	spell.cc	\
	spell.hh	\
	$(USTRING_SOURCES)

#### Debug
####

if DEBUG
DBG = test-personal-dictionary test-charset
else
DBG = 
endif

noinst_PROGRAMS = $(DBG)

## test-personal-dictionary

test_personal_dictionary_LDADD = @INTLLIBS@ $(GLIB_LIBS) test_personal_dictionary.o
test_personal_dictionary_SOURCES = \
	personal_dictionary.hh \
	common.cc common.hh $(USTRING_SOURCES) 	i18n.hh i18n.cc \
	charset.hh charset.cc tmerror.hh tmerror.cc

test_personal_dictionary.o::
	$(CXXCOMPILE) -DTEST -c -o test_personal_dictionary.o personal_dictionary.cc

## test-charset

test_charset_LDADD = @INTLLIBS@ $(GLIB_LIBS) test_charset.o
test_charset_SOURCES = \
	charset.hh tmerror.hh tmerror.cc \
	common.cc common.hh $(USTRING_SOURCES) 	i18n.hh i18n.cc
test_charset.o::
	$(CXXCOMPILE) -DTEST -c -o test_charset.o charset.cc 
