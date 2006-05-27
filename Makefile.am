MAINTAINERCLEANFILES =	Makefile.in aclocal.m4 configure missing mkinstalldirs \
			install-sh INSTALL ABOUT-NLS config.guess config.sub \
			ChangeLog autom4te.cache depcomp README LUEMINUT
			po/ChangeLog po/Makefile.in.in po/POTFILES.in \
			po/tmispell.pot

EXTRA_DIST=tmispell.conf.example tmispell.spec NEWS.fi LUEMINUT

if HAVE_ENCHANT
ENCHANT_DIR = enchant
else
ENCHANT_DIR =
endif

SUBDIRS = src $(ENCHANT_DIR) doc po
