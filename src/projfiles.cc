#include <set>
#include <iostream>
#include "projfiles.hh"

bool projfiles::selects( const boost::filesystem::path& pathname ) const {
    for( filterContainer::const_iterator filter = filters.begin();
	 filter != filters.end(); ++filter ) {
	if( boost::regex_match(pathname.string(),*filter) ) {
	    return true;
	}
    }
    return false;
}


void projfiles::dirLink( const session& s, const boost::filesystem::path& dir ) const {
    std::string href = dir.string();
    std::string topSrc = s.vars.find("topSrc")->second;
    if( href.compare(0,topSrc.size(),topSrc) == 0 ) {
	href = s.root() + dir.string().substr(topSrc.size()) + "/index.xml";
    }
    if( boost::filesystem::exists(dir.string() + "/index.xml") ) {
	std::cout << "<a href=\"" << href << "\"><h2>" 
		  << dir.leaf() << "</h2></a>" << std::endl;
    } else {
	std::cout << "<h2>" << dir.leaf() << "</h2>" << std::endl;
    }
}


void projfiles::fileLink( const session& s, const boost::filesystem::path& file ) const {
    std::string href = file.string();
    std::string topSrc = s.vars.find("topSrc")->second;
    if( href.compare(0,topSrc.size(),topSrc) == 0 ) {
	href = s.root() + file.string().substr(topSrc.size());
    }
    std::cout << "<a href=\"" << href << "\">" << file.leaf() << "</a><br />" << std::endl;
}


void projfiles::fetch( session& s, const boost::filesystem::path& pathname ) 
{
    using namespace std;
    using namespace boost::system;
    using namespace boost::filesystem;
    
    path dirname = document::root(s,pathname,"index.xml");
    
    if( !dirname.empty() ) {
	std::cout << "<div class=\"MenuWidget\">" << std::endl;

	/* We insert pathnames into a set<> such that they later can be 
	   iterated in alphabetically sorted order. */
	std::set<path> topdirs;
	std::set<path> topfiles;
	for( directory_iterator entry = directory_iterator(dirname); entry != directory_iterator(); ++entry ) {
	    if( is_directory(*entry) ) {
		topdirs.insert(*entry);
	    } else {
		if( selects(*entry) ) topfiles.insert(*entry);
	    }
	}
	
	std::cout << htmlContent;
	for( std::set<path>::const_iterator entry = topfiles.begin(); entry != topfiles.end(); ++entry ) {
	    fileLink(s,*entry);
	}
	
	for( std::set<path>::const_iterator entry = topdirs.begin(); entry != topdirs.end(); ++entry ) {
	    std::set<path> files;
	    /* Insert all filename which match a filter */
	    for( directory_iterator file = directory_iterator(*entry); 
		 file != directory_iterator(); ++file ) {
		if( !is_directory(*file) ) {
		    if( selects(*file) ) files.insert(*file);
		}
	    }
	    if( !files.empty() ) {
		dirLink(s,*entry);
		for( std::set<path>::const_iterator file = files.begin(); file != files.end(); ++file ) {
		    fileLink(s,*file);
		}
	    }
	}
	std::cout << "</div>" << std::endl;
	std::cout << std::endl;
    }
}
