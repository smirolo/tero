# -*- Makefile -*-

include $(shell dcontext)
include $(topSrc)/drop/src/prefix.mk

.PHONY: configure_apache

bins 		:=	wiki
libs		:=	libseed.a libwind.a

libseed.a: auth.o composer.o changelist.o xmltok.o cpptok.o \
	   document.o gitcmd.o projfiles.o session.o webserve.o

libwind.a: xslview.o

session.o: session.cc
	$(COMPILE.cc) -DCONFIG_FILE=\"$(shell dcontext)\" $(OUTPUT_OPTION) $<

wiki: wiki.cc libseed.a libwind.a \
	-lboost_regex -lboost_program_options \
	-lboost_filesystem -lboost_system

configure_apache:
	$(srcDir)/configure_apache

include $(topSrc)/drop/src/suffix.mk
