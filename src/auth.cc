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

#include <unistd.h>
#include <iostream>
#include <sstream>
#include <boost/date_time.hpp>
#include <boost/system/error_code.hpp>
#include <boost/filesystem/fstream.hpp>
#include "auth.hh"
#include "composer.hh"


void auth::fetch( session& s, const boost::filesystem::path& pathname ) {
    s.start();
}


void login::fetch( session& s, const boost::filesystem::path& pathname ) {
    using namespace boost::system;
    using namespace boost::posix_time;
    using namespace boost::filesystem;
    
    session::variables::const_iterator credentials = s.vars.find("credentials");
    if( s.username.empty() ) {
	throw invalidAuthentication();
    }

    s.start();
    
    /* \todo generate a random unique session id */
    uint64_t sessionId = 12345678;
    s.id = sessionId;
    s.store();
    
    /* Set a session cookie */
    std::stringstream id;
    id << sessionId;
    std::cout << cookie("session",id.str());
    std::cout << redirect(s.docAsUrl()) << '\n';
}


boost::posix_time::time_duration
deauth::aggregate( const session& s ) const {
    using namespace boost::system;
    using namespace boost::posix_time;
    using namespace boost::filesystem;

    ifstream file(s.contributorLog());
    if( file.fail() ) {
	boost::throw_exception(basic_filesystem_error<path>(
				 std::string("error opening file"),
				 s.contributorLog(), 
				 error_code()));
    }

    time_duration aggr;
    ptime start, stop, prev;
    while( !file.eof() ) {
	std::string line;
	getline(file,line);
	try {
	    size_t first = 0;
	    size_t last = line.find_first_of(' ',first);
	    last = line.find_first_of(' ',last + 1);
	    start = time_from_string(line.substr(first,last));

	    first = last + 1;
	    last = line.find_first_of(' ',first);
	    last = line.find_first_of(' ',last + 1);
	    stop = time_from_string(line.substr(first,last-first));

	    aggr += stop - start;
	    if( !prev.date().is_not_a_date() 
		&& start.date().month() != prev.date().month() ) {
		aggr = hours(aggr.hours()) 
		    + (( aggr.minutes() > 0 ) ? hours(1) : hours(0));
		std::cout << prev.date().year() << ' ' 
			  << prev.date().month() << ": " 
			  << aggr.hours() << " hours"  << std::endl;
		aggr = time_duration();
	    }
	    prev = start;
	} catch(...) {
	    /* It is ok if we cannot interpret some lines in the log;
	       most notably the last empty line. */
	}
    }
    file.close();
    aggr = hours(aggr.hours())
	+ (( aggr.minutes() > 0 ) ? hours(1) : hours(0));
    std::cout << prev.date().year() << ' ' 
	      << prev.date().month() << ": " 
	      << aggr.hours() << " hours" << std::endl;

    return aggr;
}


void deauth::fetch( session& s, const boost::filesystem::path& pathname ) {
    using namespace boost::posix_time;
    time_duration logged = s.stop();
    aggregate(s);
    std::cout << "last session ran for ";
    const char *sep = "";
    if( logged.hours() > 0 ) {
	std::cout << logged.hours() << " hours";
	sep = " ";
    }
    if( logged.minutes() > 0 ) {
	std::cout << sep << logged.minutes() << " mins";
	sep = " ";
    }
    if( strlen(sep) == 0 ) {
	std::cout << "less than a minute";
    }
    std::cout << "." << std::endl;
}


void logout::fetch( session& s, const boost::filesystem::path& pathname ) {
    using namespace boost::system;
    using namespace boost::posix_time;
    using namespace boost::filesystem;

    if( s.id != 0 ) {
	std::stringstream id;
	id << s.id;
	std::cout << cookie("session",id.str(),boost::posix_time::ptime::date_duration_type(-1));
    }
    time_duration logged = s.stop();
    std::stringstream logstr;
    logstr << logged.hours() << " hours logged." << std::endl;
    
    std::cout << htmlContent << std::endl;
#if 1
    s.vars["hours"] = logstr.str();
    path uiPath(s.vars["uiDir"] + std::string("/logout.ui"));
    composer pres(uiPath,composer::error);
    pres.fetch(s,"document");
#endif
}


