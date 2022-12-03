# Copyleft 2009-2014 Antonio Maschio <tbin at libero dot it>
#
# This file is the makefile for building taxi.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#

# Customize here
INSTALLDIR=/usr/local/bin
MANINSTALLDIR=/usr/share/man/man1/
# End of customizable items

#--------------------------------------------------
# Default objects
VERSION=`./taxi -V`
CC=gcc
CG=clang
CFLAGS=-Wall
CDFLAGS=-Wall -std=gnu99 -g --pedantic 
LIBS=-lm -lpthread
TARGET=taxi
SOURCES=taxi.c
MANPAGE=taxi.1
MANUAL=taxi.man.roff
HEADERS=taxi.h errors.h custom.h
DISTRIB=$(TARGET).`./taxi -V`.src
DOC=taxi.doc
PDFMANUAL=taxi_manual.pdf

#--------------------------------------------------
# Compile targets (must be run into taxi sources dir)
default: taxi

all:	
	taxi

taxi: 	$(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LIBS) 
	strip $(TARGET)

check: 	$(SOURCES)
	$(CC) $(CDFLAGS) -o $(TARGET) $(SOURCES) $(LIBS)

clean:
	rm -f $(TARGET) *.o

tags:
	ctags $(SOURCES)


# Uninstall/Install targets (must be run into taxi sources dir and 
# must be run by root or sudoer if destination is not under user control)
uninstall:
	rm -f $(INSTALLDIR)/$(TARGET)
	rm -f $(MANINSTALLDIR)/$(MANPAGE)

install:
	install $(TARGET) $(INSTALLDIR)
	cp $(MANPAGE) $(MANINSTALLDIR)

# doc target: build a pdf formatted textual documentation
# use it if you want to have a pdf version of your decb manual
# be sure to have ps2pdf into your system (I assume you have groff, of course)
manual:	pdfmanual
pdfmanual: 
	groff -e -t -ms -Tpdf $(MANUAL) > taxi_manual.ps 2>/dev/null
	ps2pdf14 taxi_manual.ps
	rm taxi_manual.ps

checkmanual: 
	groff -e -t -ms -Tpdf $(MANUAL) > taxi_manual.ps
	ps2pdf taxi_manual.ps
	rm taxi_manual.ps

# Distrib target - it should be run by me only, because of some tools I created
# that are present in my system but I have not distributed at all ;-)
distrib: 
	@echo "Making distribution package for $(TARGET) $(VERSION)"
	@echo "Building automatic files..."
	./taxi --greetings > README
	make pdfmanual
	@echo
	@echo "Done. Creating temporary directories..."
	mkdir $(DISTRIB)
	mkdir $(DISTRIB)/docs
	mkdir $(DISTRIB)/samples
	@echo
	@echo "Done. Copying files..."
	cp $(MANPAGE) $(SOURCES) $(HEADERS) $(DISTRIB)
	cp makefile README COPYING LICENSE taxi.vim $(DISTRIB)
	cp $(MANUAL) $(PDFMANUAL) makefile $(DISTRIB)/docs
	cp files/*.alg $(DISTRIB)/samples
	@echo
	@echo "Done. Making package..."
	zip -r vvvz $(DISTRIB)
	mv vvvz.zip $(DISTRIB).zip
	@echo
	@echo "Done. Deleting temporary directories and files..."
	rm -rf $(DISTRIB)
	@echo
	@echo "Ok, package $(DISTRIB).zip built. Time to upload it! ;-)"
# End of makefile
#
#
