/* Copyright (c) 2009-2011, Fortylines LLC
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

/** Main executable

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/

#include <unistd.h>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <iterator>
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

extern dispatchDoc semDocs;

int main( int argc, char *argv[] )
{
    using namespace std;
    using namespace boost::program_options;
    using namespace boost::filesystem;

    std::stringstream mainout;
    session s("semillaId",mainout);
    s.privileged(false);

    /* parse command line arguments */
    options_description opts;
    options_description visible;
    options_description genOptions("general");
    genOptions.add_options()
	("help","produce help message");	
    opts.add(genOptions);
    visible.add(genOptions);
    session::addSessionVars(opts,visible);
    authAddSessionVars(opts,visible);
    changelistAddSessionVars(opts,visible);
    composerAddSessionVars(opts,visible);
    postAddSessionVars(opts,visible);
    projectAddSessionVars(opts,visible);
    calendarAddSessionVars(opts,visible);
    commentAddSessionVars(opts,visible);
    
    s.restore(argc,argv,opts);
    
    bool printHelp = false;
    if( argc <= 1 ) {
	printHelp = true;
    } else {
	for( int i = 1; i < argc; ++i ) {
	    if( strncmp(argv[i],"--help",6) == 0 ) {
		printHelp = true;
	    }
	}
    }
    if( printHelp ) {
	boost::filesystem::path binname 
	    = boost::filesystem::basename(argv[0]);
	cout << binname << "[options] pathname" << endl << endl
	     << "Version" << endl
	     << "  " << binname << " version " << VERSION << endl << endl;
	cout << "Options" << endl
	     << opts << endl;
	cout << "Further Documentation" << endl
	     << "Semilla relies on session variables (ex. *siteTop*)"
	     << " to find relevent pieces of information to build"
	     << " a page."
	     << " Session variables can be defined as:\n"
	     << "   * Arguments on the command-line\n"
	     << "   * Name=Value pairs in a global configuration file\n"
	     << "   * Parameters passed through CGI environment "
	     << "variables\n"
	     << "   * Name=Value pairs in a unique session file\n"
	     << "When a variable is defined in more than one set, "
	     << "command-line arguments have precedence over the global "
	     << "configuration file which has precedence over"
	     << " environment variables which in turn as precedence"
	     << " over the session file." << endl;
	return 0;
    }
    
    cachedUrlDecorator successors(s);
    for( session::inputfilesType::const_iterator
	     inp = s.inputfiles.begin(); inp != s.inputfiles.end(); ++inp ) {
	linkLight::nexts.insert(s.asUrl(*inp));
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

    return s.errors();
}
