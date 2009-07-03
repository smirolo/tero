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

#include "download.hh"

void download::packages( const boost::filesystem::path& dirname ) {
    using namespace boost;
    using namespace boost::system;
    using namespace boost::filesystem; 
    
    for( directory_iterator entry = directory_iterator(dirname); 
	 entry != directory_iterator(); ++entry ) {
	std::cout << "<tr><td>"
		  << "<a href=\"" << *entry << "\">" 
		  << *entry << "</a></td></tr>\n";
    }
}

void download::fetch( session& s, const boost::filesystem::path& pathname ) 
{
    using namespace boost;
    using namespace boost::system;
    using namespace boost::filesystem; 

    std::cout << htmlContent;

    path dirname(s.valueOf("cacheTop"));
    std::cout << "<h1>Downloads</h1>\n";
    if( !dirname.empty() ) {
	std::cout << "<table>\n";
	for( directory_iterator entry = directory_iterator(dirname); 
	     entry != directory_iterator(); ++entry ) {
	    path packagesDir(*entry);
	    path dbFile(packagesDir);
	    dbFile /= "db.xml";
	    if( is_directory(packagesDir) && exists(dbFile) ) {
		packages(packagesDir);
	    }
	}
	std::cout << "</table>\n";
    }
}
