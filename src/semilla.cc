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
#include "projfiles.hh"
#include "project.hh"
#include "logview.hh"
#include "checkstyle.hh"
#include "calendar.hh"
#include "contrib.hh"
#include "todo.hh"
#include "blog.hh"
#include "webserve.hh"
#include "payment.hh"

/** Main executable

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


#ifndef CONFIG_FILE
#error "CONFIG_FILE should be defined when compiling this file"
#endif
#ifndef SESSION_DIR
#error "SESSION_DIR should be defined when compiling this file"
#endif

const char* session::configFile = CONFIG_FILE;
const char* session::sessionDir = SESSION_DIR;


int main( int argc, char *argv[] )
{
    using namespace std;
    using namespace boost::program_options;
    using namespace boost::filesystem;

    std::stringstream mainout;
    session s("view","semillaId",mainout);
    s.privileged(false);

    try {
	/* parse command line arguments */
	options_description opts;
	options_description genOptions("general");
	genOptions.add_options()
	    ("help","produce help message")
	    ("binDir",value<std::string>(),"path to outside executables");
	
	opts.add(genOptions);
	auth::addSessionVars(opts);
	change::addSessionVars(opts);
	composer::addSessionVars(opts);
	post::addSessionVars(opts);
	logview::addSessionVars(opts);
	projindex::addSessionVars(opts);
	calendar::addSessionVars(opts);

	s.restore(argc,argv,opts);

	/* If no document is present, set document 
	   as view in order to catch default dispatch 
	   clause. */
	boost::filesystem::path index(s.valueOf("themeDir") 
				      + std::string("/index.template"));
	session::variables::const_iterator doc = s.vars.find("document");
	if( doc == s.vars.end() ) {
	    s.insert("document",s.valueOf("view"));
	}

#if 0	
	if( params.count("help") ) {
	    cout << opts << endl;
	    cout << "1. Environment variables\n"
		 << "2. Command-line arguments\n"
		 << "3. Session database\n"
		 << "4. Configuration file\n";
	    return 0;
	}
#endif	
 
	/* by default bring the index page */
	if( (s.valueOf("view").empty() || s.valueOf("view") == "/") 
	    && boost::filesystem::exists(s.valueOf("siteTop") 
					 + std::string("index.html")) ) {
	    cout << httpHeaders.location(url("index.html"));		       
	    
	} else {	    		       
	    dispatchDoc docs;

	    /* The pattern need to be inserted in more specific to more 
	       generic order since the matcher will apply each the first
	       one that yields a positive match. */	    

#if 0
	    cancel cel;        
	    change chg           ;
	    docs.add("view",boost::regex("/cancel"),cel);
	    composer edit(s.valueOf("themeDir") + std::string("/edit.ui"));
	    docs.add("view",boost::regex("/edit"),edit);
	    login li;
	    docs.add("view",boost::regex("/login"),li);
	    logout lo;
	    docs.add("view",boost::regex("/logout"),lo);
	    docs.add("view",boost::regex("/save"),chg);
#endif

	    /* Login and Logout pages generates HTML to be displayed
	       in a web browser. It is very often convinient to quickly
	       start and stop recording worked hours from the command line.
	       In that case, "work" and "rest" can be used as substitute. */
	    auth work;
	    deauth rest;
	    docs.add("view",boost::regex("work"),work);
	    docs.add("view",boost::regex("rest"),rest);

	    /* If a template file matching the document's extension
	       is present in the theme directory, let's use it
	       as a composer. */
	    std::string ext = extension(s.valueOf("view"));
	    /* substr(1) will throw an exception if we cannot find 
	     a file extension. on the other hand if we don't put
	    entry in the global scope, pointers are screwed. */
	    if( ext.empty() ) ext = ".";	    
	    path extTemplate(path(s.valueOf("themeDir")) /
			     (ext.substr(1) + ".template"));
	    composer entry(extTemplate);
	    if( boost::filesystem::exists(extTemplate) ) {
		docs.add("view",boost::regex(std::string(".*\\") + ext),
			 entry);		
	    }

	    /* The build "document" gives an overview of the set 
	       of all projects at a glance and can be used to assess 
	       the stability of the whole as a release candidate. */
	    logview logv;
	    docs.add("document",boost::regex(".*/log/"),logv);

	    /* A project dws.xml "document" file show a description,
	       commits and unit test status of a single project through 
	       a project "view". */
	    regressions rgs;
	    docs.add("regressions",boost::regex(".*regression\\.log"),rgs);
	    boost::filesystem::path regressname
		= s.valueOf("siteTop") 
		/ boost::filesystem::path("log/tests")
		/ (boost::filesystem::path(s.valueOf("document")).parent_path().filename() 
		   + std::string("-test/regression.log"));
	    s.vars["regressions"] = regressname.string();

	    /* Command to create a new project */
	    projCreate projcreate;
	    docs.add("view",boost::regex(".*/reps/.*/create"),projcreate);

	    htmlRepository checkinHist;
	    docs.add("history",boost::regex(".*dws\\.xml"),checkinHist);
	    projindex pind;
	    docs.add("document",boost::regex(".*dws\\.xml"),pind);	 

	    /* Composer for a project view */
	    composer project(s.valueOf("themeDir")
			     + std::string("/project.template"));
	    docs.add("view",boost::regex(".*dws\\.xml"),project);	    

	    /* Widget to generate a rss feed. Attention: it needs 
	       to be declared before any of the todoFilter::viewPat 
	       (i.e. todos/.+) since an rss feed exists for todo items
	       as well. */	    
	    rssRepository reps;
	    rssSummary names;
	    rssSiteAggregate agg("document"); // \todo?
	    docs.add("document",boost::regex(".*/resources/index\\.rss"),names);
	    docs.add("document",boost::regex(".*\\.git/index\\.rss"),reps);
	    docs.add("document",boost::regex("^/index\\.rss"),agg);

	    /* Composer and document for the todos index view */
	    composer todos(s.valueOf("themeDir")
			     + std::string("/todos.template"));
	    docs.add("view",todoFilter::viewPat,todos);
	    todoIndexWriteHtml todoIdxDoc;

	    std::string active("contrib/todos/active/");
	    docs.add("document",
		     boost::regex(std::string(".*") + active),
		     todoIdxDoc);

	    boost::filesystem::path 
		todoModifs(s.srcDir(active)); 
	    todoCreate todocreate(todoModifs,std::cin);
	    docs.add("document",boost::regex("/todoCreate"),todocreate);
	    todoComment todocomment(todoModifs,std::cin);
	    docs.add("view",boost::regex("/todoComment"),todocomment);
	    docs.add("document",boost::regex("/todoComment"),todocomment);
	    todoVoteAbandon tva;
	    docs.add("document",boost::regex("/todoVoteAbandon"),tva);
	    todoVoteSuccess tvs(todoModifs,"/todoVoteSuccess");
	    docs.add("document",boost::regex("/todoVoteSuccess"),tvs);

	    /* blog presentation */ 
	    composer blogs(s.valueOf("themeDir")
			   + std::string("/blog.template"));
	    blogByIntervalDate blogtext;
	    blogByIntervalTags blogtags;
	    blogDateLinks dates;
	    blogTagLinks tags;
	    struct {
		const char *name;
		boost::regex pat;
		document& doc;
	    } lines[] = {
		{ "view", boost::regex(".*/blog/.*"), blogs },
		{ "document", boost::regex(".*/blog/tags/.*"), blogtags },
		{ "document", boost::regex(".*/blog/archive/.*"), blogtext },
		{ "document", boost::regex(".*/blog/.*"), blogtext },
		{ "dates", boost::regex(".*/blog/.*"), dates },
		{ "tags", boost::regex(".*/blog/.*"), tags }
	    };
	    for( size_t i = 0; i < sizeof(lines) / sizeof(lines[0]); ++i ) {
		docs.add(lines[i].name,lines[i].pat,lines[i].doc);
	    }
	    s.vars["dates"] = s.valueOf("document");
	    s.vars["tags"] = s.valueOf("document");
    
	    /* contribution */
	    contribIdx contribIdxDoc;
	    contribCreate contribcreate;
	    docs.add("document",boost::regex(".*contrib/"),contribIdxDoc);
	    docs.add("document",boost::regex("/contribCreate"),contribcreate);

	    calendar ical;
	    docs.add("document",boost::regex(".*\\.ics"),ical);
	    
	    /* Source code "document" files are syntax-highlighted 
	       and presented inside a source.template "view" */
	    path sourceTmpl(s.valueOf("themeDir") 
			    + std::string("/source.template"));
	    composer source(sourceTmpl);

	    changediff diff;
	    docs.add("view",boost::regex(".*/diff"),source);
	    docs.add("document",boost::regex(".*/diff"),diff);

	    mailthread mt(s.out());
	    mailParser mp(mt);
	    docs.add("document",boost::regex(".*\\.eml"),mp);

	    todoWriteHtml todoItemWriteHtml;
	    docs.add("document",boost::regex(".*\\.todo"),todoItemWriteHtml);

	    linkLight leftLinkStrm(s);
	    linkLight rightLinkStrm(s);
	    cppLight leftCppStrm;
	    cppLight rightCppStrm;
	    decoratorChain leftChain;
	    decoratorChain rightChain;
	    leftChain.push_back(leftLinkStrm);
	    leftChain.push_back(leftCppStrm);
	    rightChain.push_back(rightLinkStrm);
	    rightChain.push_back(rightCppStrm);
	    
	    text cpp(leftChain,rightChain);
	    cppCheckfile cppCheck;
	    projfiles::filterContainer filters;
	    filters.push_back(boost::regex(".*\\.c"));
	    filters.push_back(boost::regex(".*\\.h"));
	    filters.push_back(boost::regex(".*\\.cc"));
	    filters.push_back(boost::regex(".*\\.hh"));
	    filters.push_back(boost::regex(".*\\.tcc"));
	    for( projfiles::filterContainer::const_iterator f = filters.begin();
		 f != filters.end(); ++f ) {
		docs.add("check",*f,cppCheck);
		docs.add("document",*f,cpp);
		docs.add("view",*f,source);		
	    }

	    shCheckfile shCheck;
	    htmlEscaper leftLinkText;
	    htmlEscaper rightLinkText;
	    text rawtext(leftLinkText,rightLinkText);
	    boost::regex shFilterPats[] = {
		boost::regex(".*\\.mk"),
		boost::regex(".*\\.py"),
		boost::regex(".*Makefile")
	    };
	    for( boost::regex *pat = shFilterPats; 
		pat != &shFilterPats[sizeof(shFilterPats)/sizeof(boost::regex)];
		 ++pat ) {
		docs.add("check",*pat,shCheck);
		docs.add("document",*pat,rawtext);
		docs.add("view",*pat,source);		
		filters.push_back(*pat);
	    }	    

	    /* We transform docbook formatted text into HTML for .book 
	       and .corp "document" files and interpret all other unknown 
	       extension files as raw text. In all cases we use a default
	       document.template interface "view" to present those files. */ 
	    composer doc(s.valueOf("themeDir") 
			   + std::string("/document.template"));
	    linkLight leftFormatedText(s);
	    linkLight rightFormatedText(s);
	    docbook formatedDoc(leftFormatedText,rightFormatedText);
	    docs.add("document",boost::regex(".*\\.book"),formatedDoc);
	    docs.add("document",boost::regex(".*\\.corp"),formatedDoc);

	    /* \todo !!! Hack for current tmpl_include implementation */
	    text formatedText(leftFormatedText,rightFormatedText);
	    docs.add("document",boost::regex(".*\\.template"),formatedText);

	    docs.add("document",boost::regex(".*"),rawtext);

	    /* Load title from the meta tags in a text file. */
	    meta title("title");
	    todoMeta titleTodo("title");
	    docbookMeta titleBook("title");
	    consMeta titleBuildLog("title","Build View");
	    docs.add("title",boost::regex(".*/log/"),titleBuildLog);
	    docs.add("title",boost::regex(".*\\.book"),titleBook);
	    docs.add("title",boost::regex(".*\\.todo"),titleTodo);
	    docs.add("title",boost::regex(".*"),title);

	    /* homepage */
	    composer homepage(index);
	    htmlRepository siteReps;
	    htmlSiteAggregate siteAgg("feed");
	    s.vars["feed"] = session::valT("/index.feed");
	    docs.add("feed",boost::regex(".*\\.git/index\\.feed"),siteReps);
	    docs.add("feed",boost::regex("^/index\\.feed"),siteAgg);
	    docs.add("view",boost::regex("/"),homepage);

	    /* default catch-all */
	    docs.add("view",boost::regex(".*"),doc);

	    /* Widget to display status of static analysis of a project 
	       source files in the absence of a more restrictive pattern. */
	    checkstyle cks(filters.begin(),filters.end());
	    docs.add("checkstyle",boost::regex(".*"),cks);
	    s.vars["checkstyle"] = s.valueOf("document");

	    /* Widget to display the history of a file under revision control
	       in the absence of a more restrictive pattern. */	   
	    changehistory diffHist;
	    docs.add("history",boost::regex(".*"),diffHist);
	    s.vars["history"] = s.valueOf("document");
	    
	    /* Widget to display a list of files which are part of a project.
	       This widget is used through different "view"s to browse 
	       the source repository. */
	    projfiles filelist(filters.begin(),filters.end());
	    docs.add("projfiles",boost::regex(".*"),filelist);
	    s.vars["projfiles"] = s.valueOf("document");

	    /* button to Amazon payment */
	    payment pay;
	    todoAdapter ta;
	    pay.add(boost::regex(".*\\.todo"),"/todoVoteSuccess",ta);
	    docs.add("payproc",boost::regex(".*"),pay);	    
	    s.vars["payproc"] = s.valueOf("document");
      	    
	    docs.fetch(s,"view");
	    if( s.runAsCGI() ) {
		std::cout << httpHeaders.contentType();
	    }
	    std::cout << mainout.str();
	}
#if 0
	/* find a way to get .template in main() top scope. */
    } catch( exception& e ) {

	try {
	    std::cerr << "caught exception: " << e.what() << std::endl;
	    s.vars["exception"] = e.what();
	    composer except(s.valueOf("themeDir") 
			    + std::string("/exception.template"));
	    except.fetch(s,s.valueOf("document"));
#endif
	} catch( exception& e ) {
	    /* Something went really wrong if we either get here. */
	    cout << httpHeaders;
	    cout << "<html>" << endl;
	    cout << "<head>" << endl;
	    cout << "<TITLE>It is really bad news...</TITLE>" << endl;
	    cout << "</head>" << endl;
	    cout << "<body>" << endl;
	    cout << "<p>" << endl;
	    cout << "caught exception: " << e.what() << endl;
	    cout << "</p>" << endl;
	    cout << "</body>" << endl;
	    cout << "</html>" << endl;
#if 0
	}
#endif
	return 1;
    }

    return 0;
}
