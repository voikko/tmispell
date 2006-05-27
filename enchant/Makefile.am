MAINTAINERCLEANFILES = config.hh.in config.hh Makefile.in .deps stamp-h.in

target_lib = libenchant_voikko.la

INCLUDES=-I$(top_srcdir)/src $(ENCHANT_CFLAGS)

voikko_LTLIBRARIES = $(target_lib)
voikkodir = $(libdir)/enchant

USTRING_SOURCES = \
        $(top_srcdir)/src/glibmm/convert.cc \
        $(top_srcdir)/src/glibmm/exception.cc \
        $(top_srcdir)/src/glibmm/error.cc \
        $(top_srcdir)/src/glibmm/unicode.cc \
        $(top_srcdir)/src/glibmm/ustring.cc \
        $(top_srcdir)/src/glibmm/utility.cc

TMISPELL_SOURCES = \
	$(top_srcdir)/src/charset.cc \
	$(top_srcdir)/src/charset.hh \
	$(top_srcdir)/src/common.cc \
	$(top_srcdir)/src/common.hh \
	$(top_srcdir)/src/config_file.hh \
	$(top_srcdir)/src/config_file.cc \
	$(top_srcdir)/src/spell.cc \
	$(top_srcdir)/src/spell.hh \
	$(top_srcdir)/src/i18n.cc \
	$(top_srcdir)/src/i18n.hh \
	$(top_srcdir)/src/regexp.cc \
	$(top_srcdir)/src/regexp.hh \
	$(top_srcdir)/src/tmerror.cc \
	$(top_srcdir)/src/tmerror.hh

libenchant_voikko_la_LIBADD= $(ENCHANT_LIBS) $(ENCHANT_LIBS)
libenchant_voikko_la_LDFLAGS = -version-info $(VERSION_INFO) -no-undefined
libenchant_voikko_la_SOURCES = \
	voikko_provider.cc $(TMISPELL_SOURCES) $(USTRING_SOURCES)
libenchant_voikko_lalibdir=$(libdir)/enchant
