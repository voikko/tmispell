MAINTAINERCLEANFILES =	Makefile.in aclocal.m4 configure missing mkinstalldirs \
			install-sh INSTALL ABOUT-NLS config.guess config.sub \
			ChangeLog autom4te.cache depcomp README README.fi
			po/ChangeLog po/Makefile.in.in po/POTFILES.in \
			po/tmispell-voikko.pot

EXTRA_DIST=tmispell.conf.example README.fi

ACLOCAL_AMFLAGS = -I m4

if HAVE_ENCHANT
ENCHANT_DIR = enchant
else
ENCHANT_DIR =
endif

dist_man_MANS = doc/tmispell.1 doc/tmispell.conf.5

SUBDIRS = src $(ENCHANT_DIR) po
