target_lib = libenchant_voikko.la

INCLUDES=$(ENCHANT_CFLAGS)

voikko_LTLIBRARIES = $(target_lib)
voikkodir= $(libdir)/enchant

libenchant_voikko_la_LIBADD= $(ENCHANT_LIBS)
libenchant_voikko_la_LDFLAGS = -lvoikko -version-info 1:0:0 -no-undefined
libenchant_voikko_la_SOURCES = voikko_provider.cc
libenchant_voikko_lalibdir=$(libdir)/enchant

