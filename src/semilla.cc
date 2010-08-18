/* Copyright (c) 2009, Sebastien Mirolo
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

   THIS SOFTWARE IS PROVIDED BY Sebastien Mirolo ''AS IS'' AND ANY
   EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL Sebastien Mirolo BE LIABLE FOR ANY
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
#include "changelist.hh"
#include "composer.hh"
#include "docbook.hh"
#include "projfiles.hh"
#include "project.hh"
#include "logview.hh"
#include "invoices.hh"
#include "checkstyle.hh"
#include "calendar.hh"
#include "contrib.hh"
#include "todo.hh"
#include "webserve.hh"
#include "payment.hh"

#if 1
/* We use this flag to trigger features that are currently in development. */
#define devsite
#endif


class dservicesAdapter : public adapter {
public:
    virtual article fetch( session& s, 
			   const boost::filesystem::path& pathname ) {
	return article("dservices",
		       "configuration through tero",25);
    }
};


int main( int argc, char *argv[] )
{
    using namespace std;
    using namespace boost::program_options;
    using namespace boost::filesystem;

    session s;

    try {
	/* parse command line arguments */
	variables_map params;
	options_description genOptions("general");
	genOptions.add_options()
	    ("help","produce help message")
	    ("config",value<std::string>(),"load configuration file");

	options_description authOptions("authentication");
	authOptions.add_options()
	    ("credentials",value<std::string>(),"credentials")
	    ("document",value<std::string>(),"document")
	    ("right",value<std::string>(),"commit tag for right pane of diff")
	    ("session",value<std::string>(),"session")
	    ("username",value<std::string>(),"username")
	    ("message,m",value<std::string>(),"message")
	    ("view",value<std::string>(),"view")
	    ("href",value<std::string>(),"href")
	    ("client",value<std::string>(),"client")	    
	    ("editedText",value<std::string>(),"text submitted after an online edit");

	positional_options_description pd; 
	pd.add("view", 1);

	options_description calOptions("calendar");
	calOptions.add_options()
	    ("month",value<std::string>(),"month");

	options_description postOptions("posts");
	postOptions.add_options()
	    ("title",value<std::string>(),"title")
	    ("author",value<std::string>(),"author")
	    ("descr",value<std::string>(),"descr");

	options_description opts;
	opts.add(genOptions).add(authOptions).add(postOptions).add(calOptions);
	char *pathInfo = getenv("PATH_INFO");
	if( pathInfo != NULL ) {	    
	    store(parse_cgi_options(opts),params);
	    
	} else {
	    /* There is no PATH_INFO environment variable
	       so we might be running the application
	       as a non-cgi from the command line. */
	    command_line_parser parser(argc, argv);
	    opts.add(session::pathOptions);
	    parser.options(opts).positional(pd);
	    store(parser.run(),params);
	}
	
	if( params.count("help") ) {
	    cout << opts << endl;
	    return 0;
	}
	
	s.restore(params);

	/* by default bring the index page */
	if( s.vars["view"].empty()
	    || s.vars["view"] == "/" ) {
	    cout << redirect("index.html") << htmlContent << endl;
	    
	} else {	    		       
	    gitcmd revision(s.valueOf("binDir") + "/git");
	    dispatchDoc docs(s.valueOf("srcTop"));

	    /* The pattern need to be inserted in more specific to more 
	       generic order since the matcher will apply each the first
	       one that yields a positive match. */	    

#if 1
	    composer invoice(std::cout,s.valueOf("themeDir") 
			     + std::string("/invoice.template"),
			     composer::create);
	    statement stmt(std::cout);
	    docs.add("document",boost::regex("/statement"),stmt);	 
	    docs.add("view",boost::regex("/statement"),invoice);	    

	    cancel cel(std::cout);
	    change chg(std::cout);
	    docs.add("view",boost::regex("/cancel"),cel);
	    composer edit(std::cout,
			  s.vars["themeDir"] + std::string("/edit.ui"),
			  composer::create);
	    docs.add("view",boost::regex("/edit"),edit);
	    login li(std::cout);
	    docs.add("view",boost::regex("/login"),li);
	    logout lo(std::cout);
	    docs.add("view",boost::regex("/logout"),lo);
	    docs.add("view",boost::regex("/save"),chg);
#endif

	    /* Login and Logout pages generates HTML to be displayed
	       in a web browser. It is very often convinient to quickly
	       start and stop recording worked hours from the command line.
	       In that case, "work" and "rest" can be used as substitute. */
	    auth work(std::cout);
	    deauth rest(std::cout);
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
	    composer entry(std::cout,extTemplate,composer::error);
	    if( boost::filesystem::exists(extTemplate) ) {
		docs.add("view",boost::regex(std::string(".*\\") + ext),
			 entry);		
	    }

	    /* The build "document" gives an overview of the set 
	       of all projects at a glance and can be used to assess 
	       the stability of the whole as a release candidate. */
	    logview logv(std::cout);
	    docs.add("document",boost::regex(".*/log/"),logv);

	    /* A project dws.xml "document" file show a description,
	       commits and unit test status of a single project through 
	       a project "view". */
	    regressions rgs(std::cout);
	    docs.add("regressions",boost::regex(".*regression\\.log"),rgs);
	    boost::filesystem::path regressname
		= s.valueOf("siteTop") 
		/ boost::filesystem::path("log/tests")
		/ (boost::filesystem::path(s.valueOf("document")).parent_path().filename() 
		   + std::string("-test/regression.log"));
	    s.vars["regressions"] = regressname.string();

	    changedescr checkinHist(std::cout);
	    docs.add("history",boost::regex(".*dws\\.xml"),checkinHist);
	    projindex pind(std::cout);
	    docs.add("document",boost::regex(".*dws\\.xml"),pind);	 

	    /* Composer for a project view */
	    composer project(std::cout,s.valueOf("themeDir")
			     + std::string("/project.template"),
			     composer::error);
	    docs.add("view",boost::regex(".*dws\\.xml"),project);	    


	    /* Composer and document for the todos index view */
	    composer todos(std::cout,s.valueOf("themeDir")
			     + std::string("/todos.template"),
			     composer::error);
	    docs.add("view",todoFilter::viewPat,todos);
	    todoIndexWriteHtml todoIdxDoc(std::cout);

	    std::string active("contrib/todos/active/");
	    docs.add("document",
		     boost::regex(std::string(".*") + active),
		     todoIdxDoc);

	    boost::filesystem::path 
		todoModifs(s.srcDir(active)); 
	    todoCreate todocreate(todoModifs,std::cout,std::cin);
	    docs.add("document",boost::regex("/todoCreate"),todocreate);
	    todoComment todocomment(todoModifs,std::cout,std::cin);
	    docs.add("view",boost::regex("/todoComment"),todocomment);
	    docs.add("document",boost::regex("/todoComment"),todocomment);
	    todoVoteAbandon tva(std::cout);
	    docs.add("document",boost::regex("/todoVoteAbandon"),tva);
	    todoVoteSuccess tvs(todoModifs,"/todoVoteSuccess",std::cout);
	    docs.add("document",boost::regex("/todoVoteSuccess"),tvs);

	    contribIdx contribIdxDoc(std::cout);
	    contribCreate contribcreate(std::cout);
	    docs.add("document",boost::regex(".*contrib/"),contribIdxDoc);
	    docs.add("view",boost::regex("/contribCreate"),contribcreate);

	    calendar ical(std::cout);
	    docs.add("document",boost::regex(".*\\.ics"),ical);
	    
	    /* Source code "document" files are syntax-highlighted 
	       and presented inside a source.template "view" */
	    path sourceTmpl(s.valueOf("themeDir") 
			    + std::string("/source.template"));

	    changediff diff(std::cout,sourceTmpl);
	    docs.add("view",boost::regex("/diff"),diff);

	    mailthread mt(std::cout);
	    mailParser mp(std::cout,mt);
	    docs.add("document",boost::regex(".*\\.eml"),mp);

	    todoWriteHtml todoItemWriteHtml(std::cout);
	    docs.add("document",boost::regex(".*\\.todo"),todoItemWriteHtml);

	    composer source(std::cout,sourceTmpl,composer::error);
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
	    
	    text cpp(std::cout,leftChain,rightChain);
	    cppCheckfile cppCheck(std::cout);
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

	    shCheckfile shCheck(std::cout);
	    htmlEscaper leftLinkText;
	    htmlEscaper rightLinkText;
	    text rawtext(std::cout,leftLinkText,rightLinkText);
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
	    composer doc(std::cout,s.valueOf("themeDir") 
			   + std::string("/document.template"),
			   composer::error);
	    linkLight leftFormatedText(s);
	    linkLight rightFormatedText(s);
	    docbook formatedDoc(std::cout,leftFormatedText,rightFormatedText);
	    docs.add("document",boost::regex(".*\\.book"),formatedDoc);
	    docs.add("document",boost::regex(".*\\.corp"),formatedDoc);

	    /* \todo !!! Hack for current tmpl_include implementation */
	    text formatedText(std::cout,leftFormatedText,rightFormatedText);
	    docs.add("document",boost::regex(".*\\.template"),formatedText);
	    /* To display pages with embeded html forms. */
	    composer payText(std::cout,composer::error);
	    docs.add("document",boost::regex(".*\\.buy"),payText);
	    docs.add("document",boost::regex(".*"),rawtext);

#ifdef devsite
	    /* We insert advertisement in non corporate pages so we need
	       to use a different template composer for corporate pages. */
	    composer corporate(std::cout,s.valueOf("themeDir")
			       + std::string("/corporate.template"),
			       composer::error);
	    docs.add("view",boost::regex(".*\\.corp"),corporate);
#endif
	    /* Widget to generate a rss feed. */
	    changerss rss(std::cout);
	    docs.add("view",boost::regex(".*index\\.rss"),rss);

	    docs.add("view",boost::regex(".*"),doc);

	    /* Widget to display status of static analysis of a project 
	       source files in the absence of a more restrictive pattern. */
	    checkstyle cks(std::cout,filters.begin(),filters.end());
	    docs.add("checkstyle",boost::regex(".*"),cks);
	    s.vars["checkstyle"] = s.valueOf("document");

	    /* Widget to display the history of a file under revision control
	       in the absence of a more restrictive pattern. */	   
	    changehistory diffHist(std::cout);
	    docs.add("history",boost::regex(".*"),diffHist);
	    s.vars["history"] = s.valueOf("document");
	    
	    /* Widget to display a list of files which are part of a project.
	       This widget is used through different "view"s to browse 
	       the source repository. */
	    projfiles filelist(std::cout,filters.begin(),filters.end());
	    docs.add("projfiles",boost::regex(".*"),filelist);
	    s.vars["projfiles"] = s.valueOf("document");

	    /* button to Amazon payment */
	    payment pay(std::cout);
	    todoAdapter ta;
	    dservicesAdapter da;
	    pay.add(boost::regex(".*\\.todo"),"/todoVoteSuccess",ta);
	    pay.add(boost::regex(".*\\.buy"),"/dservices",da);
	    docs.add("aws",boost::regex(".*"),pay);	    
	    s.vars["aws"] = s.valueOf("document");
      	    
	    docs.fetch(s,"view");
	}
#if 0
	/* find a way to get .template in main() top scope. */
    } catch( exception& e ) {

	try {
	    std::cerr << "caught exception: " << e.what() << std::endl;
	    s.vars["exception"] = e.what();
	    composer except(s.valueOf("themeDir") 
			    + std::string("/exception.template"),
			    composer::error);
	    except.fetch(s,s.valueOf("document"));
#endif
	} catch( exception& e ) {
	    /* Something went really wrong if we either get here. */
	    cout << htmlContent << endl;
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
