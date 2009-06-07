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
#include "cppcode.hh"
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
	    cout << redirect("index") << htmlContent << endl;
	    
	} else {	    
	    path uiPath(s.vars["uiDir"] + std::string(s.exists() ? 
						      "/maintainer.ui" : "/browser.ui"));
	    dispatch docs(s.vars["topSrc"]);
	    login li;
	    logout lo;
	    composer edit(s.vars["uiDir"] + std::string("/edit.ui"),
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
	    
	    xslview xslt;
	    docs.add("document",boost::regex(".*\\.xslt"),xslt);
	    
	    linkLight leftLinkText(s);
	    linkLight rightLinkText(s);
	    text rawtext(leftLinkText,rightLinkText);
	    docs.add("document",boost::regex(".*"),rawtext);
	    
	    docs.add("view",boost::regex("/cancel"),cel);
	    docs.add("view",boost::regex("/edit"),edit);
	    docs.add("view",boost::regex("/login"),li);
	    docs.add("view",boost::regex("/logout"),lo);
	    docs.add("view",boost::regex("/save"),chg);
	    docs.add("view",boost::regex(".*"),pres);
	    
	    s.show(cerr);			
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
