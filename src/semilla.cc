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

char todoExt[] = "todo";
char corpExt[] = "corp";
char rssExt[] = "rss";
char blogExt[] = "blog";
char project[] = "project";
char docPage[] = "document";
char todos[] = "todos";
char blogPat[] = ".*\\.blog";
char except[] = "exception";
char indexPage[] = "index";
char feed[] = "feed";
char source[] = "source";
char title[] = "title";
char buildView[] = "Build View";

std::string active("contrib/todos/active/");


/* The pattern need to be inserted in more specific to more 
   generic order since the matcher will apply each the first
   one that yields a positive match. */
fetchEntry entries[] = {
    { "check", boost::regex(".*\\.c"), whenFileExist, checkfileFetch<cppChecker> },
    { "check", boost::regex(".*\\.h"), whenFileExist, checkfileFetch<cppChecker> },
    { "check", boost::regex(".*\\.cc"), whenFileExist, checkfileFetch<cppChecker> },

    { "check", boost::regex(".*\\.hh"), whenFileExist, checkfileFetch<cppChecker> },
    { "check", boost::regex(".*\\.tcc"), whenFileExist, checkfileFetch<cppChecker> },
    { "check", boost::regex(".*\\.mk"), whenFileExist, checkfileFetch<shChecker> },
    { "check", boost::regex(".*\\.py"), whenFileExist, checkfileFetch<shChecker> },
    { "check", boost::regex(".*Makefile"), whenFileExist, checkfileFetch<shChecker> },

    /* Widget to display status of static analysis of a project 
       source files in the absence of a more restrictive pattern. */
    { "checkstyle", boost::regex(".*"), always, checkstyleFetch },

    { "dates", boost::regex(".*/blog/.*"), always, blogDateLinks<blogPat> },

    /* The build "document" gives an overview of the set 
       of all projects at a glance and can be used to assess 
       the stability of the whole as a release candidate. */
    { "document", boost::regex(".*/log/"), always, logviewFetch },

    { "document", boost::regex(".*\\.c"), whenFileExist, cppFetch },
    { "document", boost::regex(".*\\.h"), whenFileExist, cppFetch },
    { "document", boost::regex(".*\\.cc"), whenFileExist, cppFetch },
    { "document", boost::regex(".*\\.hh"), whenFileExist, cppFetch },
    { "document", boost::regex(".*\\.tcc"), whenFileExist, cppFetch },

    { "document", boost::regex(".*\\.c/diff"), always, cppDiff },
    { "document", boost::regex(".*\\.h/diff"), always, cppDiff },
    { "document", boost::regex(".*\\.cc/diff"), always, cppDiff },
    { "document", boost::regex(".*\\.hh/diff"), always, cppDiff },
    { "document", boost::regex(".*\\.tcc/diff"), always, cppDiff },

    { "document", boost::regex(".*\\.mk"), whenFileExist, shFetch },
    { "document", boost::regex(".*\\.py"), whenFileExist, shFetch },
    { "document", boost::regex(".*Makefile"), whenFileExist, shFetch },

    { "document", boost::regex(".*\\.mk/diff"), always, shDiff },
    { "document", boost::regex(".*\\.py/diff"), always, shDiff },
    { "document", boost::regex(".*Makefile/diff"), always, shDiff },


    { "document", boost::regex(".*dws\\.xml"),always, projindexFetch },

    /* Widget to generate a rss feed. Attention: it needs 
       to be declared before any of the todoFilter::viewPat 
       (i.e. todos/.+) since an rss feed exists for todo items
       as well. */	    
    { "document", boost::regex(".*\\.git/index\\.rss"), always,
      feedRepository<rsswriter> },
    { "document", boost::regex(".*/index\\.rss"), always,
      feedAggregate<rsswriter,docPage> },

    { "document", boost::regex(std::string(".*") + active), always,
      todoIndexWriteHtmlFetch },

    { "document", boost::regex(".*/todoCreate"), always, todoCreateFetch },
    { "document", boost::regex(".*\\.todo/comment"), always, todoCommentFetch },
    { "document", boost::regex(".*\\.todo/voteAbandon"), always, todoVoteAbandonFetch },
    { "document", boost::regex(".*\\.todo/voteSuccess"), always, todoVoteSuccessFetch },

    { "document", boost::regex(".*/blog/tags/.*"), always, blogByIntervalTags },
    { "document", boost::regex(".*/blog/archive/.*"), always, blogByIntervalDate },
    { "document", boost::regex(".*/blog/.*"), always, blogByIntervalDate },

    /* contribution */
    { "document", boost::regex(".*contrib/"), always, contribIdxFetch },
    { "document", boost::regex(".*contrib/create"), always, contribCreateFetch },
        
    { "document", boost::regex(".*\\.commit"), always, changeShowDetails },
    { "document", boost::regex(".*\\.eml"), always, mailParserFetch },
    { "document", boost::regex(".*\\.ics"), whenFileExist, calendarFetch },
    { "document", boost::regex(".*\\.todo"), always, todoWriteHtmlFetch },

    /* We transform docbook formatted text into HTML for .book 
       and .corp "document" files and interpret all other unknown 
       extension files as raw text. In all cases we use a default
       document.template interface "view" to present those files. */ 
    { "document", boost::regex(".*\\.book"), whenFileExist, docbookFetch },
    { "document", boost::regex(".*\\.corp"), whenFileExist, docbookFetch },

    /* \todo !!! Hack for current tmpl_include implementation */
    { "document", boost::regex(".*\\.template"), whenFileExist, formattedFetch },

    { "document", boost::regex(".*"), whenFileExist, textFetch },

    /* homepage */
    { "feed", boost::regex(".*\\.git/index\\.feed"), always, feedRepositoryPopulate },
    { "feed", boost::regex("/"),always, htmlSiteAggregate<feed> },

    { "history", boost::regex(".*dws\\.xml"), always, feedRepository<htmlwriter> },
    /* Widget to display the history of a file under revision control
       in the absence of a more restrictive pattern. */	   
    { "history", boost::regex(".*"), always, changehistoryFetch },

    /* Widget to display a list of files which are part of a project.
       This widget is used through different "view"s to browse 
       the source repository. */
    { "projfiles", boost::regex(".*"), always, projfilesFetch },

    /* A project dws.xml "document" file show a description,
       commits and unit test status of a single project through 
       a project "view". */
    { "regressions", boost::regex(".*dws\\.xml"), whenFileExist, regressionsFetch },

    { "tags", boost::regex(".*/blog/.*"), always, blogTagLinks<blogPat> },

    /* Load title from the meta tags in a text file. */
    { "title", boost::regex(".*/log/"),   always, consMeta<buildView> },
    { "title", boost::regex(".*\\.book"), whenFileExist, docbookMeta },
    { "title", boost::regex(".*\\.corp"), whenFileExist, docbookMeta },
    { "title", boost::regex(".*\\.todo"), whenFileExist, todoMeta },
    { "title", boost::regex(".*\\.template"), whenFileExist, textMeta<title> },
    { "title", boost::regex(".*dws\\.xml"), whenFileExist, projectTitle },
    { "title", boost::regex(".*"), whenFileExist,metaFetch<title> },

#if 0
    { "view", boost::regex("/cancel"), always, cancelFetch },
    { "view", boost::regex("/edit"), always, compose<"edit.ui"> },
    { "view", boost::regex("/login"),always, loginFetch },
    { "view", boost::regex("/logout"),always, logoutFetch },
    { "view", boost::regex("/save"),always, changeFetch },
#endif

    /* Login and Logout pages generates HTML to be displayed
       in a web browser. It is very often convinient to quickly
       start and stop recording worked hours from the command line.
       In that case, "work" and "rest" can be used as substitute. */
    { "view", boost::regex("work"), always, authFetch },
    { "view", boost::regex("rest"), always, deauthFetch },
    
    /* If a template file matching the document's extension
       is present in the theme directory, let's use it
       as a composer. */
    { "view", boost::regex(".*\\.todo"), always, compose<todoExt> },
    { "view", boost::regex(".*\\.todo/comment"), always, todoCommentFetch },
    { "view", boost::regex(".*\\.corp"), always, compose<corpExt> },
    { "view", boost::regex(".*\\.rss"), always, compose<rssExt> },
    { "view", boost::regex(".*\\.blog"), always, compose<blogExt> },

    { "view", boost::regex(".*\\.c"), whenFileExist, compose<source> },
    { "view", boost::regex(".*\\.h"), whenFileExist, compose<source> },
    { "view", boost::regex(".*\\.cc"), whenFileExist, compose<source> },
    { "view", boost::regex(".*\\.hh"), whenFileExist, compose<source> },
    { "view", boost::regex(".*\\.tcc"), whenFileExist, compose<source> },

    { "view", boost::regex(".*\\.mk"), whenFileExist, compose<source> },
    { "view", boost::regex(".*\\.py"), whenFileExist, compose<source> },
    { "view", boost::regex(".*Makefile"), whenFileExist, compose<source> },

    /* Command to create a new project */
    { "view", boost::regex(".*/reps/.*/create"), always, projCreateFetch },

    /* Composer for a project view */
    { "view", boost::regex(".*dws\\.xml"), always, compose<project> },

    /* Composer and document for the todos index view */
    { "view", boost::regex(".*todos/.+"), always, compose<todos> },

    /* comments */
    { "view", boost::regex(std::string(".*/comment")), always, commentPage },

    /* blog presentation */ 
    { "view", boost::regex(".*/blog/.*"), always, compose<blogExt> },
    
    /* Source code "document" files are syntax-highlighted 
       and presented inside a source.template "view" */    
    { "view", boost::regex(".*/diff"), always, compose<source> },

    { "view", boost::regex("/"),always, compose<indexPage> },

    /* default catch-all */
    { "view",boost::regex(".*"), always, compose<docPage> },

#if 0
    /* button to Amazon payment */    
    { "payproc", boost::regex(".*"), always, paymentFetch },
#endif    
};




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
	
	if( !s.runAsCGI() ) {
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
	}

	/* by default bring the index page */
	if( (document.value(s).empty() || document.value(s) == "/") 
	    && boost::filesystem::exists(siteTop.value(s) 
					 / std::string("index.html")) ) {
	    cout << httpHeaders.location(url("index.html"));		       
	    
	} else {	    		       
	    dispatchDoc docs(entries,sizeof(entries)/sizeof(entries[0]));
	    docs.fetch(s,document.name);
	    if( s.runAsCGI() ) {
		std::cout << httpHeaders
		    .contentType()
		    .status(s.errors() ? 404 : 0);
	    }
	    std::cout << mainout.str();
	}

    } catch( exception& e ) {
	try {
	    std::cerr << e.what() << std::endl;
	    s.insert("exception",e.what());
	    compose<except>(s,document.name);
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
