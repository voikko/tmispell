#!/bin/sh

perl -pi -e 's,<(glibmm/.*)>,"$1",g' *.cc *.h private/*.h
perl -pi -e 's,<(glibmmconfig.h)>,"glibmm/$1",g' *.cc *.h private/*.h
perl -pi -e 's,^.*wrap_init.*$,,' *.cc
perl -pi -e 's,^#undef.*$,,' glibmmconfig.h
