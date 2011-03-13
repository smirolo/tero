# Copyright (c) 2009-2011, Fortylines LLC
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
include $(makeHelperDir)/prefix.mk

bins 		:=	semilla
#bins 		:=	semilla smailui
etcs		:=	semilla.conf
libs		:=	libsemilla.a libpayproc.a
# \todo documentation specific to the project is currently broken. 
#       It needs to be written up anyway :).
#shares		:=	semilla.pdf

semillaConfFile		?=	/etc/semilla.conf
semillaSessionDir	?=	/var/semilla

#CPPFLAGS	+=	-DREADONLY

libsemillaObjs	:= 	auth.o blog.o booktok.o calendar.o changelist.o \
			checkstyle.o comments.o composer.o contrib.o \
			cppfiles.o cpptok.o \
			docbook.o document.o feeds.o hreftok.o revsys.o \
			logview.o mail.o markup.o project.o \
			post.o session.o shfiles.o shtok.o todo.o webserve.o \
			xmlesc.o xmltok.o

libpayprocObjs	:=	aws.o payment.o paypal.o	

libsemilla.a: $(libsemillaObjs)

libpayproc.a: $(libpayprocObjs)

# On Ubuntu jaunty, libpam.a requires this library. 
# This is not the case on Ubuntu lucid.
LDFLAGS		+=	-ldl

semilla: semilla.cc libsemilla.a libpayproc.a \
		-lcryptopp -luriparser -lpam \
		-lboost_date_time -lboost_regex -lboost_program_options \
		-lboost_filesystem -lboost_system
	$(LINK.cc) -DVERSION=\"$(version)\" -DCONFIG_FILE=\"$(semillaConfFile)\" -DSESSION_DIR=\"$(semillaSessionDir)\" $(filter %.cc %.a %.so,$^) $(LOADLIBES) $(LDLIBS) -o $@


smailui: smailui.cc libsemilla.a \
		-lcryptopp -luriparser -lpam \
		-lboost_date_time -lboost_regex -lboost_program_options \
		-lboost_filesystem -lboost_system
	$(LINK.cc) -DCONFIG_FILE=\"$(semillaConfFile)\" -DSESSION_DIR=\"$(semillaSessionDir)\" $(filter %.cc %.a %.so,$^) $(LOADLIBES) $(LDLIBS) -o $@

semilla.conf: $(shell dws context)
	echo "binDir=/var/www/cgi-bin" > $@
	echo "siteTop=/var/www" >> $@
	echo "srcTop=/var/www/reps" >> $@
	echo "remoteIndexFile=$(remoteIndexFile)" >> $@
	echo "themeDir=$(shareDir)/semilla/default" >> $@

semilla.fo: $(call bookdeps,$(srcDir)/doc/semilla.book)

include $(makeHelperDir)/suffix.mk

# the installation of this executable is special because we need
# to dynamically change ownership in order to execute admin commands.
#install:: semilla
#	/usr/bin/install -s -p -m 4755 -o root $< $(binDir)

install:: $(wildcard $(srcDir)/data/themes/default/*)
	$(installDirs) $(shareDir)/semilla
	cp -Rf $(srcDir)/data/themes $(shareDir)/semilla

install:: $(srcDir)/src/semilla.pam
	$(installDirs) $(etcDir)/pam.d
	$(installFiles) $^ $(etcDir)/pam.d/$(basename $(notdir $^))
