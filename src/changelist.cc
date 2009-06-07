#include <cstdio>
#include "changelist.hh"


void cancel::fetch( session& s, const boost::filesystem::path& pathname ) {
	std::cout << redirect(s.docAsUrl()) << '\n';
}


void change::fetch( session& s, const boost::filesystem::path& pathname ) {
    using namespace boost::system;
    using namespace boost::filesystem;

	session::variables::const_iterator document = s.vars.find("document");
	session::variables::const_iterator text = s.vars.find("editedText");
	if( text != s.vars.end() ) {
		path docName(s.vars["topSrc"] + document->second + std::string(".edits")); 

		if( !exists(docName) ) {
		    create_directories(docName);
		}

		ofstream file(docName);
		if( file.fail() ) {
			boost::throw_exception(basic_filesystem_error<path>(std::string("unable to open file"),
																docName, 
																error_code()));
		}
		file << text->second;
		file.close();

		/* add entry in the changelist */
		path changesPath(s.userPath().string() + std::string("/changes"));
		ofstream changes(changesPath,std::ios::app);
		if( file.fail() ) {
		    boost::throw_exception(basic_filesystem_error<path>(std::string("unable to open file"),
									changesPath, 
									error_code()));
		}
		file << docName;
		file.close();
    }
	std::cout << redirect(s.docAsUrl() + std::string(".edits")) << '\n';
}


void changediff::embed( session& s, const std::string& varname ) {
	using namespace std;

	if( varname != "document" ) {
		composer::embed(s,varname);
	} else {
		std::stringstream text;
		std::string leftRevision = s.valueOf("left");
		std::string rightRevision = s.valueOf("right");

#if 0
		boost::filesystem::path docname(s.valueOf(varname).substr(1));
#else
		boost::filesystem::path docname(s.valueOf("topSrc") 
										+ s.valueOf(varname));
		cerr << "docname: " << docname << std::endl;
		boost::filesystem::path gitrelname = document::relativePath(docname,revision->rootpath);
#endif
		revision->diff(text,leftRevision,rightRevision,gitrelname);
		
		cout << "<table>" << endl;
		cout << "<tr>" << endl;
		cout << "<td>" << leftRevision << "</td>" << endl;
		cout << "<td>" << rightRevision << "</td>" << endl;
		cout << "</tr>" << endl;

		boost::filesystem::ifstream input;
		open(input,docname);

		/* \todo the session is not a parameter to between files... */	
		document *doc = dispatch::instance->select("document",docname.string());
		((::text*)doc)->showSideBySide(input,text,false);
		
		cout << "</table>" << endl;
		input.close();
	}
}


void changelist::fetch( session& s, const boost::filesystem::path& pathname ) {
    using namespace boost::system;
    using namespace boost::filesystem;
    /* retrieve the active changelist */
    path changesPath(s.userPath().string() + std::string("/changes"));
    ifstream changes(changesPath);
	bool empty = true;
    if( !changes.fail() ) {
		std::string line;
		std::getline(changes,line);
		while( !changes.eof() ) {
			path p(line);
			std::cout << p.leaf() << "<br />" << std::endl;
			std::getline(changes,line);
			empty = false;
		}
		changes.close();
	}
    if( empty ) {
	std::cout << "<i>empty</i><br />" << std::endl;
    }
}


void 
changehistory::fetch( session& s, const boost::filesystem::path& pathname ) 
{
    std::stringstream text;
    boost::filesystem::path sccsRoot = document::root(s,pathname,".git");
    if( !sccsRoot.empty() ) {
	revision->rootPath(sccsRoot);
	revision->history(text,pathname);
	std::cout << "<div class=\"MenuWidget\">" << std::endl;
	std::cout << "<p>history</p>" << std::endl;

	while( !text.eof() ) {
	    std::string line;
	    getline(text,line);
	    /* Parse the summary line in order to split the commit tag 
	       from the commit message. */
	    
	    size_t splitPos = line.find(' ');
	    if( splitPos != std::string::npos ) {
		std::string rightRevision = line.substr(0,splitPos);
		std::string title = line.substr(splitPos);
		
		/* \todo '\n' at end of line? */
		std::cout << "<a";
		std::cout << " href=\"/dev/bin/wiki/diff?document=/" << s.docAsUrl() 
			  << "&right=" << rightRevision << "\""; 		
		std::cout << " title=\"" << title << "\"";
		std::cout << ">";
		std::cout << rightRevision.substr(0,10);
		std::cout << "...</a>";
	    }
	}
	std::cout << std::endl;
	std::cout << "</div>" << std::endl;
	std::cout << std::endl;
    }
}

