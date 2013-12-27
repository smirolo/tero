# Copyright (c) 2009-2013, Fortylines LLC
#   All rights reserved.
#
#   Redistribution and use in source and binary forms, with or without
#   modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of fortylines nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
#   THIS SOFTWARE IS PROVIDED BY Fortylines LLC ''AS IS'' AND ANY
#   EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#   DISCLAIMED. IN NO EVENT SHALL Fortylines LLC BE LIABLE FOR ANY
#   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# -*- Makefile -*-

include $(shell dws context)
version :=  0.4
include $(buildTop)/share/dws/prefix.mk


CXXFLAGS    += -g

bins        :=  semilla
libs        :=  libsemilla.a
# \todo documentation specific to the project is currently broken. 
#       It needs to be written up anyway :).
#shares		:=	semilla.pdf
etcs        := $(srcDir)/etc/pam.d/semilla.pam

semillaConfFile ?=  $(etcDir)/semilla/default.conf
sessionDir      ?=  $(LOCALSTATEDIR)/db/semilla

#CPPFLAGS	+=	-DREADONLY

libsemillaObjs	:= blog.o booktok.o calendar.o changelist.o \
			checkstyle.o contrib.o composer.o \
			cppfiles.o cpptok.o coverage.o \
			docbook.o document.o errtok.o feeds.o hreftok.o revsys.o \
			logview.o mail.o markdown.o markup.o project.o \
			post.o rfc2822tok.o rfc5545tok.o session.o shfiles.o shtok.o \
			todo.o webserve.o \
			xmlesc.o xmltok.o

libsemilla.a: $(libsemillaObjs)

# On Ubuntu jaunty, libpam.a requires this library.
# This is not the case on Ubuntu lucid.
LDFLAGS		+=	-ldl

#registerDeps	:= -lpam -lldap

semilla: semilla.cc semtable.o libsemilla.a \
		-lcryptopp -luriparser \
		-lboost_date_time -lboost_random -lboost_regex -lboost_program_options \
		-lboost_iostreams -lboost_filesystem -lboost_system -lPocoNet
	$(LINK.cc) -DVERSION=\"$(version)\" -DCONFIG_FILE=\"$(semillaConfFile)\" -DSESSION_DIR=\"$(sessionDir)\" $(filter %.cc %.o %.a %.so,$^) $(LOADLIBES) $(LDLIBS) -o $@ $(registerDeps)

semilla.fo: $(call bookdeps,$(srcDir)/doc/semilla.book)

include $(buildTop)/share/dws/suffix.mk

# the installation of this executable is special because we need
# to dynamically change ownership in order to execute admin commands.
#install:: semilla
#	/usr/bin/install -s -p -m 4755 -o root $< $(binDir)

install:: $(wildcard $(srcDir)/data/themes/default/*)
	$(installDirs) $(DESTDIR)$(shareDir)/semilla
	cp -Rf $(srcDir)/data/themes $(DESTDIR)$(shareDir)/semilla

install:: $(srcDir)/etc/semilla/default.conf
	$(if $(findstring /etc/semilla,$(semillaConfFile)),\
		$(installDirs) $(DESTDIR)$(dir $(semillaConfFile)))
	$(if $(findstring /etc/semilla,$(semillaConfFile)),\
		$(installFiles) $< $(DESTDIR)$(semillaConfFile))
