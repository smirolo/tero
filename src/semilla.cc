/* Copyright (c) 2009-2013, Fortylines LLC
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of fortylines nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY Fortylines LLC ''AS IS'' AND ANY
   EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL Fortylines LLC BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#include <unistd.h>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "auth.hh"
#include "feeds.hh"
#include "changelist.hh"
#include "composer.hh"
#include "docbook.hh"
#include "project.hh"
#include "logview.hh"
#include "checkstyle.hh"
#include "calendar.hh"
#include "comments.hh"
#include "contrib.hh"
#include "todo.hh"
#include "blog.hh"
#include "webserve.hh"
#include "payment.hh"
#include "cppfiles.hh"
#include "shfiles.hh"

/** Main executable

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


#ifndef CONFIG_FILE
#error "CONFIG_FILE should be defined when compiling this file"
#endif
#ifndef SESSION_DIR
#error "SESSION_DIR should be defined when compiling this file"
#endif
#ifndef VERSION
#error "VERSION should be defined when compiling this file"
#endif

const char* session::configFile = CONFIG_FILE;
const char* session::sessionDir = SESSION_DIR;

char except[] = "exception";
extern dispatchDoc semDocs;

int main( int argc, char *argv[] )
{
    using namespace std;
    using namespace boost::program_options;
    using namespace boost::filesystem;

    std::stringstream mainout;
    session s("semillaId",mainout);
    s.privileged(false);

    try {
		/* parse command line arguments */
		options_description genOptions("caching");
		genOptions.add_options()
			("cache","produce a static cache out of the dynamic content");
		s.opts.add(genOptions);
		s.visible.add(genOptions);
		docAddSessionVars(s.opts,s.visible);
		authAddSessionVars(s.opts,s.visible);
		changelistAddSessionVars(s.opts,s.visible);
		composerAddSessionVars(s.opts,s.visible);
		postAddSessionVars(s.opts,s.visible);
		projectAddSessionVars(s.opts,s.visible);
		calendarAddSessionVars(s.opts,s.visible);
		commentAddSessionVars(s.opts,s.visible);
		logAddSessionVars(s.opts,s.visible);
	
		s.restore(argc,argv);
		
		bool genCache = false;
		if( !s.runAsCGI() ) {
			for( int i = 1; i < argc; ++i ) {
				if( strncmp(argv[i],"--cache",7) == 0 ) {
					genCache = true;
				}
			}
		}
		
		if( genCache ) {
            /* XXX root is first link in set. */
			cachedUrlDecorator successors(s, s.abspath(*s.inputs.begin()));
			for( session::inputsType::const_iterator
					 inp = s.inputs.begin(); inp != s.inputs.end(); ++inp ) {
				linkLight::nexts.insert(*inp);
			}
			
			while( !linkLight::empty() ) {
				linkLight::clear();
				for( linkLight::linkSet::const_iterator l = linkLight::begin();
					 l != linkLight::end(); ++l ) {
					try {
						s.reset();
						s.insert(document.name,l->string(),session::cmdline);
						boost::filesystem::path cached
							= s.absCacheName(document.value(s));
						cout << "generating " << cached << " (for "
							 << *l << ") ..." << endl;
						/* \todo view != document, need to clear all session
						   variables except the ones loaded from config file. */
						boost::filesystem::ofstream out;
						s.createfile(out,cached);
						successors.attach(out);
						s.out(out);
						semDocs.fetch(s,document.name,document.value(s));
						successors.detach();
						out.close();
					} catch( exception& e ) {
						cerr << "error: " << e.what() << endl;
						++s.nErrs;
					}
				}
			}
		}  else {
			/* When we run as CGI, we will assume the path is a url relative
			   to siteTop while running in shell command-line mode, we will
			   assume it is a regular filename. */
			semDocs.fetch(s,document.name,document.value(s));
			if( s.runAsCGI() ) {
				cout << httpHeaders
					.contentType()
					.status(s.errors() ? 404 : 0);
			}
			cout << mainout.str();
		}
		
    } catch( exception& e ) {
		try {
			std::cerr << e.what() << std::endl;
			s.insert("exception",e.what());
			compose<except>(s,document.value(s));
		} catch( exception& e ) {
			/* Something went really wrong if we either get here. */
			cout << httpHeaders.contentType().status(404);
			cout << "<html>" << endl;
			cout << html::head() << endl
				 << "<title>It is really bad news...</title>" << endl
				 << html::head::end << endl;
			cout << html::body() << endl << html::p() << endl
				 << "caught exception: " << e.what() << endl
				 << html::p::end << endl << html::body::end << endl;
			cout << "</html>" << endl;
		}
		++s.nErrs;
    }

    return s.errors();
}
