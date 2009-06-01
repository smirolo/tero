#include <iostream>
#include <boost/regex.hpp>
#include <boost/filesystem/fstream.hpp>
#include "composer.hh"


void composer::embed( session& s, const std::string& value ) {
	dispatch::instance->fetch(s,value);
}


void composer::fetch( session& s, const boost::filesystem::path& pathname ) {
    using namespace boost;
    using namespace boost::system;
    using namespace boost::filesystem;
    
    std::cerr << "embed..." << std::endl; 
    static const boost::regex embed("<a href=\"#(\\S+)\">.*</a>");
    std::cerr << "link..." << std::endl; 
    static const boost::regex link("\\$\\{(\\S+)\\}");

    std::cerr << "composer::fetch..." << std::endl;     
    ifstream strm;
    open(strm,fixed.empty() ? pathname : fixed);
    std::cout << htmlContent;
    while( !strm.eof() ) {
		smatch m;
		std::string line;
		bool found = false;
		std::getline(strm,line);
		if( regex_search(line,m,embed) ) {
			session::variables::const_iterator v = s.vars.find(m.str(1));
			if( v != s.vars.end() ) {
				std::cout << m.prefix();
				this->embed(s,m.str(1));
				std::cout << m.suffix() << std::endl;
				found = true;
			}
		} else if( regex_search(line,m,link) ) {
			session::variables::const_iterator v = s.vars.find(m.str(1));
			std::cout << m.prefix() 
					  << ((v != s.vars.end()) ? v->second : m.str(1)) 
					  << m.suffix() << std::endl;
			found = true;
		}
		if( !found ) {
			std::cout << line << std::endl;
		}
    }
    strm.close();
}
