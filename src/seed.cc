/* Copyright (c) 2009, Sebastien Mirolo
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of codespin nor the
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

#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "auth.hh"
#include "changelist.hh"
#include "composer.hh"
#include "download.hh"
#include "projfiles.hh"
#include "xslview.hh"
#include "webserve.hh"


int main( int argc, char *argv[] )
{
    using namespace std;
    using namespace boost::program_options;
    using namespace boost::filesystem;

#if 0
    /* This code is used to debug initial permission problems. */
    struct passwd *pw;
    struct group *grp;
    
    pw = getpwuid(getuid());
    grp = getgrgid(getgid());
    cerr << "real      : " << getuid() 
	 << '(' << pw->pw_name << ")\t" << getgid() 
	 << '(' << grp->gr_name << ')' << endl;
    pw = getpwuid(geteuid());
    grp = getgrgid(getegid());
    cerr << "effective : " << geteuid() 
	 << '(' << pw->pw_name << ")\t" << getegid() 
	 << '(' << grp->gr_name << ')' << endl;
#endif

    try {
	/* parse command line arguments */
	variables_map params;
	options_description	authOptions("authentication");
	authOptions.add_options()
	    ("help","produce help message")
	    ("credentials",value<std::string>(),"credentials")
	    ("document",value<std::string>(),"document")
	    ("right",value<std::string>(),"commit tag for right pane of diff")
	    ("session",value<std::string>(),"session")
	    ("username",value<std::string>(),"username")
	    ("view",value<std::string>(),"view")
	    ("editedText",value<std::string>(),"text submitted after an online edit");
	positional_options_description pd; 
	pd.add("view", 1);
	
	char *pathInfo = getenv("PATH_INFO");
	if( pathInfo != NULL ) {
	    store(parse_cgi_options(authOptions),params);
	    
	} else {
	    /* There is no PATH_INFO environment variable
	       so we might be running the application
	       as a non-cgi from the command line. */
	    command_line_parser parser(argc, argv);
	    parser.options(authOptions).positional(pd);
	    store(parser.run(),params);
	}
	
	if( params.count("help") ) {
	    cout << authOptions << endl;
	    return 0;
	}
	
	session s;
	s.restore(params);

#if 0		
	/* topUrl is used as the root for all redirects. */
	char *base = getenv("REQUEST_URI");
	if( base != NULL ) {
	    std::cerr << "REQUEST_URI=" << base;
	    char *endPtr;
	    for( endPtr = base; *endPtr != '\0' & *endPtr != '?' & *endPtr != '#'; ++endPtr );
	    endPtr -= s.vars["view"].size();
	    assert( endPtr - base > 0 );
	    s.vars["topUrl"] = std::string(base,endPtr - base);
	} else {
	    s.vars["topUrl"] = std::string();
	}
#endif

	/* by default bring the index page */
	if( s.vars["view"].empty()
	    || s.vars["view"] == "/" ) {
	    cout << redirect("corporate/index.book") << htmlContent << endl;
	    
	} else {	    
	    path uiPath(s.vars["themeDir"] + std::string(s.exists() ? 
						      "/maintainer.ui" : "/entry.html"));
	    dispatch docs(s.vars["srcTop"]);
	    login li;
	    logout lo;
	    composer edit(s.vars["themeDir"] + std::string("/edit.ui"),
			  composer::create);
	    composer pres(uiPath,composer::error);
	    cancel cel;
	    change chg;
	    
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
	    projfiles::filterContainer filters;
	    filters.push_back(boost::regex(".*\\.c"));
	    filters.push_back(boost::regex(".*\\.h"));
	    filters.push_back(boost::regex(".*\\.cc"));
	    filters.push_back(boost::regex(".*\\.hh"));
	    filters.push_back(boost::regex(".*\\.tcc"));
	    for( projfiles::filterContainer::const_iterator f = filters.begin();
		 f != filters.end(); ++f ) {
		docs.add("document",*f,cpp);
	    }
	    
	    changelist cl;
	    docs.add("session",boost::regex(".*"),cl);
	    
	    filters.push_back(boost::regex(".*Makefile"));			
	    projfiles filelist(filters.begin(),filters.end());
	    docs.add("projfiles",boost::regex(".*"),filelist);
	    s.vars["projfiles"] = s.vars["document"];
	    
	    gitcmd revision(s.valueOf("binDir") + "/git");
	    changehistory history(&revision);
	    docs.add("history",boost::regex(".*"),history);
	    s.vars["history"] = s.vars["document"];
	    
	    changediff diff(uiPath,&revision);
	    docs.add("view",boost::regex("/diff"),diff);

	    /* The pattern need to be inserted in more specific to more 
	       generic order since the matcher will apply each the first
	       one that yields a positive match. */
	    download dlcmd;
	    docs.add("document",boost::regex("/download"),dlcmd);
	    
	    xslview xslt;
	    docs.add("document",boost::regex(".*\\.xslt"),xslt);

	    linkLight leftFormatedText(s);
	    linkLight rightFormatedText(s);
	    text formatedText(leftFormatedText,rightFormatedText);
	    docs.add("document",boost::regex(".*\\.book"),formatedText);
	    
	    htmlEscaper leftLinkText;
	    htmlEscaper rightLinkText;
	    text rawtext(leftLinkText,rightLinkText);
	    docs.add("document",boost::regex(".*"),rawtext);

	    docs.add("view",boost::regex("/cancel"),cel);
	    docs.add("view",boost::regex("/edit"),edit);
	    docs.add("view",boost::regex("/login"),li);
	    docs.add("view",boost::regex("/logout"),lo);
	    docs.add("view",boost::regex("/save"),chg);
	    docs.add("view",boost::regex(".*"),pres);
	    
	    docs.fetch(s,"view");
	}
	
    } catch( exception& e ) {
	cout << htmlContent << endl;
	cout << "<html>" << endl;
	cout << "<head>" << endl;
	cout << "<TITLE>Response</TITLE>" << endl;
	cout << "</head>" << endl;
	cout << "<body>" << endl;
	cout << "<p>" << endl;
	cout << e.what() << endl;
	cout << "</p>" << endl;
	cout << "</body>" << endl;
	cout << "</html>" << endl;
	return 1;
    }

    return 0;
}
