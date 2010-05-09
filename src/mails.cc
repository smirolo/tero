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
#include "mails.hh"
#include "markup.hh"

namespace {

    boost::posix_time::ptime from_mbox_string( const std::string& s ) {
	boost::posix_time::ptime t;
#if 0
	std::stringstream dts;
	::boost::posix_time::time_input_facet* input_facet 
	      = new ::boost::posix_time::time_input_facet;
	dts.imbue(std::locale(dts.getloc(), input_facet));
	input_facet->format("%a, %e %b %Y %T");
	dts.str(s);
	dts >> t;
#else
	size_t p = s.find(',') + 1;
	/* day of the month */
	while( s[p] == ' ' ) ++p;
	int day = s[p++] - '0';
	if( s[p] >= '0' && s[p] <= '9' ) day = day * 10 + s[p++] - '0';
	/* month */
	int month = 0;
	while( s[p] == ' ' ) ++p;
	if( s.compare(p,7,"January") == 0 ) {
	    month = 1;
	} else if( s.compare(p,8,"February") == 0 ) {
	    month = 2;
	} else if( s.compare(p,5,"March") == 0 ) {
	    month = 3;
	} else if( s.compare(p,5,"April") == 0 ) {
	    month = 4;
	} else if( s.compare(p,3,"May") == 0 ) {
	    month = 5;
	} else if( s.compare(p,4,"June") == 0 ) {
	    month = 6;
	} else if( s.compare(p,4,"July") == 0 ) {
	    month = 7;
	} else if( s.compare(p,6,"August") == 0 ) {
	    month = 8;
	} else if( s.compare(p,9,"September") == 0 ) {
	    month = 9;
	} else if( s.compare(p,7,"October") == 0 ) {
	    month = 10;
	} else if( s.compare(p,8,"November") == 0 ) {
	    month = 11;
	} else if( s.compare(p,8,"December") == 0 ) {
	    month = 12;
	}
	/* 4-digits year */
	while( s[p] != ' ' ) ++p;
	while( s[p] == ' ' ) ++p;
	int year = (s[p++] - '0') * 1000;
	year += (s[p++] - '0') * 100;
	year += (s[p++] - '0') * 10;
	year += (s[p++] - '0');

	/* time formatted as 24-hour:minutes:seconds */
	while( s[p] == ' ' ) ++p;
	int hours = (s[p++] - '0') * 10;
	hours += (s[p++] - '0');
	assert( s[p++] == ':' );

	while( s[p] == ' ' ) ++p;
	int minutes = (s[p++] - '0') * 10;
	minutes += (s[p++] - '0');
	assert( s[p++] == ':' );
	
	while( s[p] == ' ' ) ++p;
	int seconds = (s[p++] - '0') * 10;
	seconds += (s[p++] - '0');

	t = boost::posix_time::ptime(boost::gregorian::date(year,month,day),
		   boost::posix_time::time_duration(hours,minutes,seconds)); 
#endif
	return t;
    }

} // anonymous namespace


bool mailthread::filters( const post& p ) const {
    return true;
}


void mailthread::fetch( session& s, const boost::filesystem::path& pathname )
{
    using namespace boost::gregorian;
    using namespace boost::posix_time;
    using namespace boost::local_time;
    using namespace std;


    static const boost::regex metainfo("^(\\S+):(.+)");

    boost::filesystem::ifstream infile;
    open(infile,pathname);

    post p;
    bool skip = false;
    bool first = true;
    std::stringstream descr;
    while( !infile.eof() ) {
	boost::smatch m;
	std::string line;
	std::getline(infile,line);

	if( line.compare(0,5,"From ") == 0 ) {
	    /* Beginning of new message
	       http://en.wikipedia.org/wiki/Mbox */
	    if( !first ) {
		p.descr = descr.str();
		descr.str("");
		if( filters(p) ) {
		    p.expanded(std::cout);
		}
	    }
	    p = post();
	    first = false;

	} else if( line.compare(0,5,"Date:") == 0 ) {
	    p.time = from_mbox_string(line.substr(5));
	} else if( line.compare(0,5,"From:") == 0 ) {
	    p.author = strip(line.substr(5));
	} else if( line.compare(0,9,"Subject: ") == 0 ) {
	    p.title = strip(line.substr(9));
	} else if( line.compare(0,9,"Received:") == 0 ) {
	    skip = true;
	} else if( regex_match(line,m,metainfo) ) {
	    /* This is more meta information we donot interpret */
	} else {
	    if( !skip ) descr << line << std::endl;
	    skip = false;
	}
    }

    if( !first ) {
	p.descr = descr.str();
	descr.str("");
	if( filters(p) ) {
	    p.expanded(std::cout);
	}
    }

    infile.close();
}

/*
  <h2></h2>
  <h3></h3>
  <h3>not-a-date-time</h3>
  <p>
  From smirolo@willy.localdomain  Thu May  6 14:24:02 2010
  </p>

  <h2></h2>
  <h3></h3>
  <h3>not-a-date-time</h3>
  <p>
  id 939B0E637A; Thu,  6 May 2010 14:24:02 -0700 (PDT)
  </p>

  <h2></h2>
  <h3></h3>
  <h3>not-a-date-time</h3>
  <p>
  Hello
  From smirolo@willy.localdomain  Thu May  6 14:29:12 2010
  </p>

  <h2></h2>
  <h3></h3>
  <h3>not-a-date-time</h3>
  <p>
  id 804CFE63A9; Thu,  6 May 2010 14:29:12 -0700 (PDT)
  </p>
 */
