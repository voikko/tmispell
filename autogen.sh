#!/bin/sh -e

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


## Generate documentation
make -C doc/sgml -f Makefile.am generate
cp -f doc/tmispell.en.txt README
cp -f doc/tmispell.fi.txt LUEMINUT
ln -fs tmispell.en.1.in doc/tmispell.1.in

## Gettext.
find src -name '*.cc' -o -name '*.hh' | sort > po/POTFILES.in
glib-gettextize -f

## Autoconf+etc.
libtoolize --force --copy
aclocal -I src/glibmm/m4
autoheader
automake-1.9 --add-missing --copy --force-missing --foreign
autoconf
autoheader

