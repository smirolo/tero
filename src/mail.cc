/* Copyright (c) 2009-2011, Fortylines LLC
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

   THIS SOFTWARE IS PROVIDED BY Fortylines LLC ''AS IS'' AND ANY
   EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL Fortylines LLC BE LIABLE FOR ANY
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

/** Mail parser

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


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


void mailParser::walk( session& s, std::istream& ins, const std::string& name ) const
{
    using namespace boost::gregorian;
    using namespace boost::posix_time;
    using namespace boost::local_time;
    using namespace std;

    typedef std::set<std::string> tagSet;

    /** tags associated to a post. 
     */
    tagSet tags;

    static const boost::regex metainfo("^(\\S+):(.+)");

    post p;
    bool first = true;
    size_t lineCount = 0;
    std::stringstream descr;
    parseState state;

    p.score = 0;
    p.filename = boost::filesystem::path(name);
    p.guid = std::string("/") + s.subdirpart(s.valueOf("siteTop"),name).string();	   
    descr << html::pre();

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
		descr << html::pre::end;
		p.descr = descr.str();
		descr.str("");
		descr << html::pre();
		p.normalize();
		if( tags.empty() ) {
		    filter->filters(p);
		} else {
		    for( tagSet::const_iterator t = tags.begin(); t != tags.end(); ++t ) {
			p.tag = *t;
			filter->filters(p);
		    }
		}
		tags.clear();
	    }
	    p = post();
	    p.filename = boost::filesystem::path(name);
	    p.guid = std::string("/") + s.subdirpart(s.valueOf("siteTop"),name).string();
	    p.score = 0;
	    first = false;
	    state = startParse;

	} else if( line.compare(0,5,"Date:") == 0 ) {
	    try {
		p.time = from_mbox_string(line.substr(5));
	    } catch( std::exception& e ) {
	    }
	    state = dateParse;
	} else if( line.compare(0,5,"From:") == 0 ) {
	    p.authorEmail = line.substr(5);
	    /* \todo As long as we donot parse name separately. */
	    p.authorName = line.substr(5);
	    state = authorParse;
	} else if( line.compare(0,9,"Subject: ") == 0 ) {
	    p.title = line.substr(9);
	    state = titleParse;
	} else if( line.compare(0,7,"Score: ") == 0 ) {
	    p.score = atoi(line.substr(7).c_str());

	} else if( line.compare(0,6,"Tags: ") == 0 ) {
	    size_t first = 6, last = 6;
	    while( last != line.size() ) {
		if( line[last] == ',' ) {
		    std::string s = strip(line.substr(first,last-first));
		    if( ! s.empty() ) tags.insert(s);
		    first = last + 1;
		}
		++last;
	    }
	    std::string s = strip(line.substr(first,last-first));
	    if( ! s.empty() ) tags.insert(s);

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
    descr << html::pre::end;
    p.descr = descr.str();
    descr.str("");
    p.normalize();
    if( tags.empty() ) {
	filter->filters(p);
    } else {
	for( tagSet::const_iterator t = tags.begin(); t != tags.end(); ++t ) {
	    p.tag = *t;
	    filter->filters(p);
	}
    }
    tags.clear();
}


void mailParserFetch( session& s, const boost::filesystem::path& pathname )
{
    mailthread mt(s.out());
    mailParser mp(mt);
    mp.fetch(s,pathname);
#if 0
    /* \todo Cannot flush here if we want to support feedContent calls blogEntry.
       This call will generate non-intended intermediate flushes. It might be
       useful for other parts of the system though...
     */
    filter->flush();
#endif
}
