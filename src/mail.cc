/* Copyright (c) 2009, Sebastien Mirolo
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of fortylines nor the
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

#include <string>
#include <locale>
#include <boost/date_time/local_time/local_time.hpp>
#include "mail.hh"
#include "markup.hh"


void mailthread::filters( const post& p ) {
    static const boost::regex titlePat("(^.*:)(.*)");

    boost::smatch m;
    if( boost::regex_match(p.title,m,titlePat) ) {    
	indexMap::iterator found = indexes.find(m.str(2));
	if( found != indexes.end() ) {
	    ++found->second;
	} else {
	    indexes[m.str(2)] = 0;
	}
    }
}


void mailthread::flush() {
    for( indexMap::iterator idx = indexes.begin(); 
	 idx != indexes.end(); ++idx ) {
	*ostr << idx->first << "(" << idx->second << ")" << std::endl;
    }
}


void mailParser::fetch( session& s, const boost::filesystem::path& pathname )
{
    dirwalker::fetch(s,pathname);
    filter->flush();
}


void mailParser::walk( session& s, std::istream& ins, const std::string& name )
{
    using namespace boost::gregorian;
    using namespace boost::posix_time;
    using namespace boost::local_time;
    using namespace std;

    static const boost::regex metainfo("^(\\S+):(.+)");

    post p;
    bool first = true;
    size_t lineCount = 0;
    std::stringstream descr;
    parseState state;

    p.score = 0;
    p.guid = boost::filesystem::path(boost::filesystem::path(name).filename()).stem();

    while( !ins.eof() ) {
	boost::smatch m;
	std::string line;
	std::getline(ins,line);
	++lineCount;

	if( line.compare(0,5,"From ") == 0 ) {
	    /* Beginning of new message
	       http://en.wikipedia.org/wiki/Mbox */	    
	    if( !first ) {
		if( stopOnFirst ) break;
		p.descr = descr.str();
		descr.str("");
#if 0
		std::cerr << "new post at line " << lineCount << std::endl;
#endif
		p.normalize();
		filter->filters(p);
	    }
	    p = post();
	    p.guid = boost::filesystem::path(boost::filesystem::path(name).filename()).stem();
	    p.score = 0;
	    first = false;
	    state = startParse;

	} else if( line.compare(0,5,"Date:") == 0 ) {
	    try {
		p.time = from_mbox_string(line.substr(5));
	    } catch( std::exception& e ) {
#if 0
		std::cerr << "!!! exception " << e.what() << std::endl; 
#endif
	    }
	    state = dateParse;
	} else if( line.compare(0,5,"From:") == 0 ) {
	    p.authorEmail = line.substr(5);
	    state = authorParse;
	} else if( line.compare(0,9,"Subject: ") == 0 ) {
	    p.title = line.substr(9);
	    state = titleParse;
	} else if( line.compare(0,7,"Score: ") == 0 ) {
	    p.score = atoi(line.substr(7).c_str());

	} else if( regex_match(line,m,metainfo) ) {
	    /* This is more meta information we donot interpret */

	} else if( line.size() > 0 
		   && ((line[0] == ' ') | (line[0] == '\t')) ) {
	    /* field spans multiple lines */
	    switch( state ) {
	    case startParse:
		break;
	    case dateParse:
		break;
	    case authorParse:
		p.authorEmail += line;
		break;
	    case titleParse:
		p.title += line;
		break;
	    }
	} else {
	    descr << line << std::endl;
	    state = startParse;
	}
    }
    p.descr = descr.str();
    descr.str("");
    p.normalize();
    filter->filters(p);
}

