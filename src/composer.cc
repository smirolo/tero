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
