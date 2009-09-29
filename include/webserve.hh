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

#ifndef guardwebserve
#define guardwebserve

#include <iostream>
#include <iomanip>
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

/** \file webserve.hh 

    - Common functions used by web servers to access files
    - Wrappers around common CGI functionalities.
    (See http://hoohoo.ncsa.uiuc.edu/cgi/env.html for documentation.)
*/

/** returns the latest modification time of a file given its *pathname*.
 */
boost::posix_time::ptime
getmtime( const boost::filesystem::path& pathname );

/** returns *pathname* as a relative path from *base*.
 */
boost::filesystem::path 
relpath( const boost::filesystem::path& pathname,
	 const boost::filesystem::path& base );


/** \brief Generate a cookie on the client side
 */
class cookie {
private:
    std::string name;
    std::string value;
    boost::posix_time::ptime::date_duration_type lifetime;
    
public:
    cookie( const std::string& n, const std::string& v, 
	    boost::posix_time::ptime::date_duration_type d 
	    = boost::posix_time::ptime::date_duration_type(0) ) 
	: name(n), value(v), lifetime(d) {}
    
    template<typename ch, typename tr>
    friend std::basic_ostream<ch, tr>&
    operator<<(std::basic_ostream<ch, tr>& ostr, const cookie& v ) {
	using namespace boost::posix_time;
	ptime now = second_clock::universal_time(); // Use the clock 
	// Get the date part out of the time 
	ptime::date_type today = now.date();        
	ptime::date_type expires = today + v.lifetime;
	
	ostr << "Set-Cookie:" << v.name << "=" << v.value << ';'
	     << " Path=/;";
	if( v.lifetime != boost::posix_time::ptime::date_duration_type(0) ) {
	    ostr << " expires=" 
		 << expires.day_of_week()
				  << ", " << expires.day()
		 << ' ' << expires.month()
		 << " " << std::setfill('0') << std::setw(2) << expires.year()
		 << " " << std::setfill('0') << std::setw(2) 
		 << now.time_of_day().hours()
		 << ':' << std::setfill('0') << std::setw(2) 
		 << now.time_of_day().minutes()
		 << ':' << std::setfill('0') << std::setw(2) 
		 << now.time_of_day().seconds()
		 << " GMT;";
	}
	ostr << " Version=1\r\n";
	return ostr;
    }
};


/** \brief start serving an HTML document 
 */
class htmlContentServe {
private:
    bool firstTime;

public:
    htmlContentServe();

    template<typename ch, typename tr>
    friend std::basic_ostream<ch, tr>&
    operator<<(std::basic_ostream<ch, tr>& ostr, htmlContentServe& v ) {
	if( v.firstTime ) {
	    ostr << "Content-Type:text/html;charset=iso-8859-1\r\n";
	    v.firstTime = false;
	}
	return ostr;
    }
    
};

extern htmlContentServe htmlContent;


/** \brief redirect to another webpage
 */
class redirect {
private:
    std::string url;
    
    bool absolute()  const {
	return url.compare(0,5,"http:")
	    || url.compare(0,6,"https:")
	    || url.compare(0,1,"/");
    }
    
public:
    redirect( std::string u ) 
	: url(u) {}
    
    template<typename ch, typename tr>
    friend std::basic_ostream<ch, tr>&
    operator<<(std::basic_ostream<ch, tr>& ostr, const redirect& v ) {
	ostr << "Location: " << v.url << '\n';
	return ostr;
    };
};


/* \brief url syntax as per rfc1738
 */
class url {
public:
    template<typename ch, typename tr>
    friend std::basic_ostream<ch, tr>&
    operator<<(std::basic_ostream<ch, tr>& ostr, url& u ) {
	if( u.absolute() ) ostr << u.protocol << ':';
	if( !u.host.empty() ) ostr << "//" << u.host;
	if( u.port > 0 ) ostr << ':' << u.port;
	ostr << u.pathname;
	return ostr;
    }	
	
public:

    int port;
    std::string host;
    std::string protocol;
    boost::filesystem::path pathname;
    
public:
    explicit url( const std::string& name );
    
    url( const std::string& pprotocol, const std::string& phost, int pport, 
	 const boost::filesystem::path& ppathname ) 
	: protocol(pprotocol), host(phost), port(pport), pathname(ppathname) {} 
    
    bool absolute() const;
};


/** \brief Populate options based on environment variables and stdin
 */
boost::program_options::basic_parsed_options<char>
parse_cgi_options( const boost::program_options::options_description& descr );

#endif
