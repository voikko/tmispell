MAINTAINERCLEANFILES =  Makefile.in doc.pot \
	tmispell.fi.sgml tmispell.man.fi.sgml \
	tmispell.en.sgml tmispell.man.en.sgml


EXTRA_DIST = fi.po
SGMLS=\
	tmispell.configfile-example.ent	\
	tmispell.configfile.ent		\
	tmispell.description.ent	\
	tmispell.man.sgml		\
	tmispell.sgml			\
	tmispell.todo.ent

EXTRA_DIST += $(SGMLS) $(patsubst %.sgml,%.fi.sgml,$(filter %.sgml,$(SGMLS)))

LANGS=en fi

translate:
	@for L in $(LANGS); do \
		for F in $(filter %.sgml,$(SGMLS)); do \
			B=`basename $$F .sgml`; \
			T="$$B".$$L.sgml; \
			echo "Generating translated file $$T"; \
			if [ "$$L" = en ]; then \
				ln -fs "$$F" "$$T"; \
			else \
				P="$$F".pot; \
				[ "$$F" = "$$B" ] || { \
					xmllint --noent "$$F" > "$$F".tmp; \
					pot2xml-tr fi.po "$$F".tmp | sed \
					  -e "s,\.sgml,.fi.sgml,g" \
					  > "$$T"; \
					xml2pot-tr "$$F".tmp > "$$P"; \
					rm -f "$$F".tmp; \
				}; \
			fi; \
			[ -f "$$F" -a ! -s "$$T" ] && rm -f "$$T" || true; \
		done; \
	done
	msgcat -o doc.pot *.sgml.pot 2> /dev/null
	rm -f *.sgml.pot

generate: translate
	@for L in $(LANGS); do \
		echo "Generating manual page ($$L)"; \
		docbook-to-man tmispell.man.$$L.sgml > ../tmispell.$$L.1.in; \
		echo "Generating html document ($$L)"; \
		xsltproc ../doc.xslt tmispell.$$L.sgml > ../tmispell.$$L.html; \
		tidy -m ../tmispell.$$L.html; \
		if [ $$L == "fi" ]; then \
			perl -pi -e 's/NAME/NIMI/g;' ../tmispell.$$L.1.in; \
			perl -pi -e 's/Example/Esimerkki/g;' ../tmispell.$$L.html; \
		fi; \
		echo "Generating text document ($$L)"; \
		LANG=fi_FI@euro links -dump ../tmispell.$$L.html > ../tmispell.$$L.txt; \
	done
