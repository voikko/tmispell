#!/bin/sh -e

# Automake version.
AM_VERSION_SUFFIX=

echo Cleaning autotools files...
find -type d -name autom4te.cache -print0 | xargs -0 rm -rf \;
find -type f \( -name missing -o -name install-sh -o -name mkinstalldirs \
        -o -name depcomp -o -name ltmain.sh -o -name configure \
        -o -name config.sub -o -name config.guess \
        -o -name Makefile.in \) -print0 | xargs -0 rm -f

rm -f config.sub config.guess
cp /usr/share/misc/config.sub .
cp /usr/share/misc/config.guess .
cp /usr/share/gettext/config.rpath .


## Gettext.
find src -name '*.cc' -o -name '*.hh' | sort > po/POTFILES.in
cp /usr/share/gettext/po/Makefile.in.in po/
cp /usr/share/gettext/po/remove-potcdate.sin po/

## Autoconf+etc.
libtoolize --force --copy
aclocal$AM_VERSION_SUFFIX
autoheader
automake$AM_VERSION_SUFFIX --add-missing --copy --force-missing --foreign
autoconf
autoheader

