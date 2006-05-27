Summary: Tmispell is a front-end for Ispell
Name: tmispell
Version: 0.2.3
Release: 1
License: GPL
Vendor: Pauli Virtanen
Group: Application/Text
Packager: Marko Myllynen
Source0: %{name}-%{version}.tar.gz
BuildRoot: /var/tmp/%{name}-root
BuildRequires: perl, gettext
Requires: /usr/bin/ispell, perl


%description
Tmispell is a front-end that emulates Ispell for some spell checking
libraries.  Tmispell is useful because many programs (e.g., mail clients
and document processors) use Ispell for spell checking.  So Tmispell makes
it possible to use the spell checking services provided by modules like
Soikko in these programs, with no changes to either needed.

Additionally Tmispell can launch the original Ispell instead when there is
no module for some language.  This makes it feasible to use Tmispell as a
drop-in replacement for Ispell.

%prep

%setup

%build
CFLAGS="$RPM_OPT_FLAGS" %configure
make

%install
[ $RPM_BUILD_ROOT != "/" ] && rm -rf $RPM_BUILD_ROOT

%makeinstall
mkdir -p $RPM_BUILD_ROOT%{_sysconfdir}
mkdir -p $RPM_BUILD_ROOT%{_docdir}/%{name}
mkdir -p $RPM_BUILD_ROOT/usr/lib/ispell

install -c -m 0644 tmispell.conf.example $RPM_BUILD_ROOT%{_sysconfdir}/tmispell.conf
perl -pi -e 's/ispell.real/ispell/' $RPM_BUILD_ROOT%{_sysconfdir}/tmispell.conf

install -c -m 0644 AUTHORS COPYING ChangeLog NEWS README $RPM_BUILD_ROOT%{_docdir}/%{name}

touch $RPM_BUILD_ROOT/usr/lib/ispell/finnish.aff
touch $RPM_BUILD_ROOT/usr/lib/ispell/suomi.aff

%clean
[ $RPM_BUILD_ROOT != "/" ] && rm -rf $RPM_BUILD_ROOT

%pre

%post
# Look for Soikko, Finnish spellchecker/hyphenator
if [ -x /usr/lib/libsoikko.so ]
then
	perl -pi -e 's/usr\/local\/lib\/libsoikko.so/usr\/lib\/libsoikko.so/' /etc/tmispell.conf
fi
if [ -f /usr/share/soikko/soikko-sp.fi_FI.dic ]
then
	perl -pi -e 's/usr\/local\/share\/soikko\/soikko-sp.fi_FI.dic/usr\/share\/soikko\/soikko-sp.fi_FI.dic/' $RPM_BUILD_ROOT%{_sysconfdir}/tmispell.conf
fi

# Info for user
if [ "$1" = "1" -a ! -f /usr/bin/ispell.real ]
then
	echo "To use tmispell as a drop-in replacement for ispell, do (as root):"
	echo " "
	echo "mv /usr/bin/ispell /usr/bin/ispell.real"
	echo "ln -s /usr/bin/tmispell /usr/bin/ispell"
	echo "perl -pi -e 's/bin\/ispell/bin\/ispell.real/' /etc/tmispell.conf"
fi


%preun

%postun
# Info for user
if [ "$1" = "0" -a -L /usr/bin/ispell -a -f /usr/bin/ispell.real ]
then
	echo "Please check /usr/bin/ispell and /usr/bin/ispell.real"
fi

%files
%defattr(-,root,root)
%{_bindir}/%{name}
%{_datadir}/locale/fi/LC_MESSAGES/tmispell.mo
%doc %{_docdir}/%{name}/*
%{_mandir}/man1/%{name}.1*
%config (noreplace) %{_sysconfdir}/%{name}.conf
/usr/lib/ispell/finnish.aff
/usr/lib/ispell/suomi.aff

%changelog
* Mon May 19 2003 Pauli Virtanen 0.2.3-1
- version 0.2.3

* Thu May 15 2003 Pauli Virtanen 0.2.2-1
- version 0.2.2

* Tue May 13 2003 Pauli Virtanen 0.2.1-1
- version 0.2.1

* Mon May 12 2003 Pauli Virtanen 0.2-1
- version 0.2

* Wed Oct  9 2002 Marko Myllynen 0.1.2-2
- added files /usr/lib/ispell/*.aff for KDE

* Mon Oct  7 2002 Pauli Virtanen 0.1.2-1
- version 0.1.2

* Tue Oct  1 2002 Marko Myllynen 0.1.1-1
- initial version, tmispell 0.1.1
