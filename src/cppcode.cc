#include <iostream>
#include <boost/filesystem/fstream.hpp>
#include "cppcode.hh"

#if 0
// deprecated pull interface
void cppcode::fetch( session& s, const boost::filesystem::path& pathname ) {
    using namespace boost::system;
    using namespace boost::filesystem; 
    
    ifstream strm;
    open(strm,pathname);
    
    htmlCppTokListener cppListener(std::cout);
    cppTokenizer tokenizer(cppListener);
    std::cout << htmlContent;
	std::cout << "<pre>" << std::endl;
    while( !strm.eof() ) {
		std::string line;
		std::getline(strm,line);
		tokenizer.tokenize(line.c_str());
		std::cout << std::endl;
    }
	std::cout << "</pre>" << std::endl;
    strm.close();
}
#endif
