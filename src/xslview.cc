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
