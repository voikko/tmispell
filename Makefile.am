MAINTAINERCLEANFILES =	Makefile.in aclocal.m4 configure missing mkinstalldirs \
			install-sh INSTALL ABOUT-NLS config.guess config.sub \
			ChangeLog autom4te.cache depcomp README LUEMINUT
			po/ChangeLog po/Makefile.in.in po/POTFILES.in \
			po/tmispell.pot

EXTRA_DIST=tmispell.conf.example tmispell.spec	\
	debian/changelog \
	debian/conffiles \
	debian/config \
	debian/control \
	debian/copyright \
	debian/preinst \
	debian/postinst \
	debian/postrm \
	debian/README.debian \
	debian/rules \
	debian/templates \
	debian/templates.fi \
	debian/tmispell.conf.debian \
	NEWS.fi LUEMINUT

if HAVE_ENCHANT
ENCHANT_DIR = enchant
else
ENCHANT_DIR =
endif

SUBDIRS = src $(ENCHANT_DIR) doc po
