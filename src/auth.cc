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

#include <unistd.h>
#include <iostream>
#include <sstream>
#include <boost/date_time.hpp>
#include <boost/system/error_code.hpp>
#include <boost/filesystem/fstream.hpp>
#include "auth.hh"
#include "composer.hh"

void login::fetch( session& s, const boost::filesystem::path& pathname ) {
    using namespace boost::system;
    using namespace boost::posix_time;
    using namespace boost::filesystem;
    
    session::variables::const_iterator credentials = s.vars.find("credentials");
    if( s.username.empty() ) {
	throw invalidAuthentication();
    }

    path hours(s.userPath());
    hours /= "hours";
    if( exists(hours) ) {
	std::stringstream buffer;
	buffer << "touch " << hours;
	system(buffer.str().c_str());
    } else {
	ofstream file(hours);
	if( !file.fail() ) {
	    file << time_duration(0,0,0,0) << std::endl;
	} else {
	    boost::throw_exception(basic_filesystem_error<path>(std::string("unable to open file"),
								hours, 
								error_code()));
	}
	file.close();
    }
    
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


void logout::fetch( session& s, const boost::filesystem::path& pathname ) {
    using namespace boost::system;
    using namespace boost::posix_time;
    using namespace boost::filesystem;

    if( s.id != 0 ) {
	std::stringstream id;
	id << s.id;
	std::cout << cookie("session",id.str(),boost::posix_time::ptime::date_duration_type(-1));
    }
    
    time_duration logged;
    ptime stop = second_clock::universal_time();

    path hours(s.userPath());
    hours /= "hours";
    ptime start = from_time_t(last_write_time(hours));
    fstream file(hours);
    if( file.fail() ) {
	boost::throw_exception(basic_filesystem_error<path>(
				 std::string("error opening file"),
				 hours, 
				 error_code()));
    }
    file >> logged;
    file.seekp(0);
    logged += stop - start;
    file << logged;
    file.close();

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


