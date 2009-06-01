#include <iostream>
#include <boost/regex.hpp>
#include <boost/program_options.hpp>
#include <boost/system/error_code.hpp>
#include "xslview.hh"

void xslview::fetch( session& s, const boost::filesystem::path& pathname ) {
	using namespace boost;
    using namespace boost::system;
    using namespace boost::filesystem; 
    
    ifstream strm;
    open(strm,pathname);
    std::cout << htmlContent;

    std::cerr << "xslview::fetch..." << std::endl;     

#if 0
    static const boost::regex linkTag("(<a.+)href=\"(\\S+)\"");
#else
	static const boost::regex linkTag(
	  "<\\s*A\\s+[^>]*href\\s*=\\s*\"([^\"]*)\"(\\s+style\\s*=\\s*\"([^\"]*)\")?",
	  boost::regex::normal | boost::regbase::icase);
#endif

	smatch m;
	std::string line;
	std::getline(strm,line);
    while( !strm.eof() ) {
		while( regex_search(line,m,linkTag) ) {
			std::cout << "prefix=" << m.prefix() << std::endl;
			boost::filesystem::path content = s.findFile(m.str(1));
			boost::filesystem::path style = s.findFile(m.str(3));

			std::cout << "content=" << content << std::endl;
			std::cout << "style=" << style << std::endl;

			/* Execute xsltproc command */ 
			std::stringstream sstm;
			sstm << "xsltproc " << style << " " << content; 

			char textLine[256];
			FILE *text = popen(sstm.str().c_str(),"r");
			assert( text != NULL );

			while( fgets(textLine,sizeof(textLine),text) != NULL ) {
				std::cout << textLine;
			}
			pclose(text);

			/* Keep going */
			line = m.suffix();
		}
		std::getline(strm,line);
    }
    strm.close();
}
