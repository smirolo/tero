# Copyright (c) 2009, Sebastien Mirolo
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
#   THIS SOFTWARE IS PROVIDED BY Sebastien Mirolo ''AS IS'' AND ANY
#   EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#   DISCLAIMED. IN NO EVENT SHALL Sebastien Mirolo BE LIABLE FOR ANY
#   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# -*- Makefile -*-

include $(shell dws context)
include $(etcBuildDir)/dws/prefix.mk

bins 		:=	seed
etcs		:=	seed.conf
libs		:=	libseed.a
shares		:=	seed.pdf

seedConfFile	?=	/etc/seed.conf

#CPPFLAGS	+=	-DREADONLY

libseed.a: $(subst .cc,.o,\
	      $(filter-out seed.cc session.cc,\
		$(notdir $(wildcard $(srcDir)/src/*.cc))))

seed: seed.cc session.o libseed.a libcryptopp.a liburiparser.a \
	libboost_date_time.a libboost_regex.a libboost_program_options.a \
	libboost_filesystem.a libboost_system.a

session.o: session.cc
	$(COMPILE.cc) -DCONFIG_FILE=\"$(seedConfFile)\" $(OUTPUT_OPTION) $<

seed.conf: $(shell dws context)
	echo "binDir=/var/www/bin" > $@
	echo "siteTop=/var/www" >> $@
	echo "srcTop=/var/www/reps" >> $@
	echo "remoteIndex=$(remoteIndex)" >> $@
	echo "themeDir=$(shareDir)/seed/default" >> $@

seed.fo: $(call bookdeps,$(srcDir)/doc/seed.book)

include $(etcBuildDir)/dws/suffix.mk

install:: $(wildcard $(srcDir)/data/themes/default/*)
	$(installDirs) $(shareDir)/seed
	cp -Rf $(srcDir)/data/themes $(shareDir)/seed