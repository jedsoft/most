# -*- sh -*-
#
# To create the SGML files, you will need to install the tmexpand
# package.  See http://www.jedsoft.org/ for more information.
#
TM2SGML = tmexpand
SGML2HTML = sgml2html
DOCBOOK2MAN = docbook2man
DOCBOOK2TXT = docbook2txt

all: most.1 most.txt

most.1: most.sgml
	$(DOCBOOK2MAN) most.sgml
most.sgml: most.tm
	$(TM2SGML) most.tm most.sgml
most.txt: most.sgml
	$(DOCBOOK2TXT) most.sgml
view: most.1
	nroff -man most.1 | most
#----------------------------------------------------------------------------
clean:
	-rm -f *~ *-error manpage.refs manpage.links
distclean: clean
	-rm -f most.1 most.sgml
install: most.1 most.txt
	mv most.1 most.txt ..
