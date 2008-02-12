dnl Process this file with autoconf to produce a configure script.
AC_PREREQ(2.60)

dnl ## Package information
AC_INIT([tmispell-voikko],[0.7],[hatapitk@iki.fi])
AC_CONFIG_SRCDIR(src/tmispell.cc)
AM_INIT_AUTOMAKE([1.10 foreign])
AM_CONFIG_HEADER(src/config.hh)

dnl ## Enable maintainer mode
AM_MAINTAINER_MODE

dnl ## Required compiler and other programs
AC_PROG_CXX
AC_PROG_CXXCPP
AC_PROG_INSTALL
AC_PROG_LIBTOOL

dnl ## Check CXX features
AC_LANG_CPLUSPLUS

dnl ## Language
AC_LANG(C++)

dnl ## Required header files
AC_UNISTD_H
AC_CHECK_HEADER([dlfcn.h],,
	[AC_MSG_ERROR([This program requires dlfcn.h to work.])])
AC_CHECK_HEADERS(vector,,
	[AC_MSG_ERROR([Missing header file. Do you have C++ standard library
		       installed?])])
AC_CHECK_HEADER([regex.h],,
	[AC_MSG_ERROR([This program requires regex.h to work.])])
AC_CHECK_HEADER([curses.h],,
	[AC_MSG_ERROR([This program requires curses.h to work.
		       Do you have Ncurses installed?])])
AC_CHECK_HEADER([libvoikko/voikko.h],,
	[AC_MSG_ERROR([This program requires voikko.h to work.
	               Do you have libvoikko installed?])])

PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.0.0)
PKG_CHECK_MODULES(GLIBMM, glibmm-2.4)

dnl ## Config file
if test "x${sysconfdir}" = 'x${prefix}/etc'; then
  if test "x${prefix}" = "xNONE"; then
    PACKAGE_CONF_DIR="${ac_default_prefix}/etc"
  else
    PACKAGE_CONF_DIR="${prefix}/etc"
  fi
else
  PACKAGE_CONF_DIR="${sysconfdir}"
fi

CONFIG_FILE=${PACKAGE_CONF_DIR}/tmispell.conf

AC_ARG_WITH(config-file,
	AC_HELP_STRING([--with-config-file=PATH],
                       [The configuration file]),
	[CONFIG_FILE="$withval"])

AC_DEFINE_UNQUOTED(CONFIG_FILE, "$CONFIG_FILE", [The configuration file])

AC_SUBST(CONFIG_FILE)

dnl ## Gettext
AM_GNU_GETTEXT([external])

GETTEXT_PACKAGE=$PACKAGE
AC_SUBST(GETTEXT_PACKAGE)

if test "x${prefix}" = "xNONE"; then
  PACKAGE_LOCALE_DIR="${ac_default_prefix}/share/locale"
else
  PACKAGE_LOCALE_DIR="${prefix}/share/locale"
fi
AC_DEFINE_UNQUOTED(PACKAGE_LOCALE_DIR, ["$PACKAGE_LOCALE_DIR"], [Package locale directory])
AC_SUBST(PACKAGE_LOCALE_DIR)

dnl ## Enchant
AC_ARG_ENABLE(enchant, AC_HELP_STRING([--disable-enchant],
	[disable building of the libenchant provider plugin]),
	[enchant=${enableval}], [enchant=yes])
AH_TEMPLATE(HAVE_ENCHANT, [Have enchant >= 1.0])
AH_TEMPLATE(HAVE_ENCHANT_1_1_6, [Have enchant >= 1.1.6])
HAVE_ENCHANT=""
if test x$enchant = xyes; then
	PKG_CHECK_MODULES(ENCHANT, enchant >= 1.1.6, [
	  AC_DEFINE(HAVE_ENCHANT_1_1_6, 1)
	  AC_DEFINE(HAVE_ENCHANT, 1)
	  HAVE_ENCHANT="true"
	  ],
	  [
	  PKG_CHECK_MODULES(ENCHANT, enchant >= 1.0, [
	    AC_DEFINE(HAVE_ENCHANT, 1)
	    HAVE_ENCHANT="true"
	  ])
	  ]
	)
fi
AM_CONDITIONAL(HAVE_ENCHANT, test x$HAVE_ENCHANT = xtrue)

dnl ## Debug
AC_ARG_ENABLE(debug,
	[  --enable-debug    Turn on debugging],
	[case "${enableval}" in
	  yes) debug=true ;;
	  no)  debug=false ;;
	  *) AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
	esac],[debug=false])
if test x$debug = xtrue; then
	CFLAGS="$CFLAGS -ggdb -O0"
	CXXFLAGS="$CXXFLAGS -ggdb -O0"
fi
AM_CONDITIONAL(DEBUG, test x$debug = xtrue)

dnl ## Finish
AC_OUTPUT([
Makefile
src/Makefile
enchant/Makefile
po/Makefile.in
])

dnl Print configuration
cat <<EOF

-- Configuration:

        The configuration file: $CONFIG_FILE

EOF

