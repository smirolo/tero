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

    url encode/decode:
    http://www.ietf.org/rfc/rfc2396.txt

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


/* \todo rfc? percent escape a string. */
std::string uriEncode( const std::string& );


/* \brief url syntax as per rfc1738
 */
class url {
public:
    template<typename ch, typename tr>
    friend std::basic_ostream<ch, tr>&
    operator<<(std::basic_ostream<ch, tr>& ostr, const url& u ) {
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
    url() : port(0), host(""), protocol(""), pathname("") {}

    explicit url( const std::string& name );
    
    url( const std::string& pprotocol, const std::string& phost, 
	 const boost::filesystem::path& ppathname ) 
	: port(0), host(phost), protocol(pprotocol), pathname(ppathname) {} 

    url( const std::string& pprotocol, const std::string& phost, int pport, 
	 const boost::filesystem::path& ppathname ) 
	: port(pport), host(phost), protocol(pprotocol), pathname(ppathname) {} 
    
    bool absolute() const;

    bool empty() const {
	return string().empty();
    }

    std::string string() const;
};


/**/
class httpHeaderSet {
protected:
    bool firstTime;

    std::string contentTypeValue;
    std::string contentTypeCharset;
    size_t contentLengthValue;
    std::string contentDispositionValue;
    std::string contentDispositionFilename;
    std::string setCookieName;
    std::string setCookieValue;
    boost::posix_time::ptime::date_duration_type setCookieLifetime;
    url locationValue;
    size_t refreshDelay;
    url refreshUrl;

    template<typename ch, typename tr>
    friend std::basic_ostream<ch, tr>&
    operator<<( std::basic_ostream<ch, tr>& ostr, httpHeaderSet& h ) {
	if( !h.firstTime ) return ostr;
       
	if( !h.contentTypeValue.empty() ) {
	    ostr << "Content-Type:" << h.contentTypeValue;
	    if( !h.contentTypeCharset.empty() ) {
		ostr << ";charset=" << h.contentTypeCharset;
	    }
	    ostr << "\r\n";
	}
	if( h.contentLengthValue > 0 ) {
	    ostr << "Content-Length:" << h.contentLengthValue << "\r\n";
	}
	if( !h.contentDispositionValue.empty() ) {
	    ostr << "Content-Disposition:" << h.contentDispositionValue;
	    if( !h.contentDispositionFilename.empty() ) {
		ostr << ";filename=" << h.contentDispositionFilename;
	    }
	    ostr << "\r\n";
	}
	if( !h.setCookieName.empty() ) {
	    ostr << "Set-Cookie:" << h.setCookieName << "=" << h.setCookieValue
		 << "; Path=/";
	    if( h.setCookieLifetime 
		!= boost::posix_time::ptime::date_duration_type(0) ) {
		using namespace boost::posix_time;
		ptime now = second_clock::universal_time(); // Use the clock 
		// Get the date part out of the time 
		ptime::date_type today = now.date();        
		ptime::date_type expires = today + h.setCookieLifetime;
		ostr << "; expires=" 
		     << expires.day_of_week() 
		     << ", " << expires.day()
		     << ' ' << expires.month()
		     << " " << std::setfill('0') << std::setw(2) 
		     << expires.year()
		     << " " << std::setfill('0') << std::setw(2) 
		     << now.time_of_day().hours()
		     << ':' << std::setfill('0') << std::setw(2) 
		     << now.time_of_day().minutes()
		     << ':' << std::setfill('0') << std::setw(2) 
		     << now.time_of_day().seconds()
		     << " GMT";
	    }
	    ostr << "; Version=1\r\n";
	}
	if( !h.locationValue.empty() ) {
	    ostr << "Location: " << h.locationValue << "\r\n";
	}
	if( !h.refreshUrl.empty() ) {
	    ostr << "Refresh:" << h.refreshDelay 
		 << ";URL=" << h.refreshUrl << "\r\n";
	}
	ostr << "\r\n";
	h.firstTime = false;
	return ostr;
    }

public:
    httpHeaderSet();

    httpHeaderSet& contentDisposition( const std::string& value, 
				       const std::string& filename );

    httpHeaderSet& contentType( const std::string& value = "text/html", 
				 const std::string& charset = "iso-8859-1" );

    httpHeaderSet& contentLength( size_t length );

    httpHeaderSet& location( const url& v );

    httpHeaderSet& refresh( size_t delay, const url& v );

    httpHeaderSet& 
    setCookie( const std::string& name, 
	       const std::string& value, 
	       boost::posix_time::ptime::date_duration_type expires 
	       = boost::posix_time::ptime::date_duration_type(0) );
};

extern httpHeaderSet httpHeaders;


class emptyParaHackType {
public:
    template<typename ch, typename tr>
    friend std::basic_ostream<ch, tr>&
    operator<<(std::basic_ostream<ch, tr>& ostr, const emptyParaHackType& v ) {
	ostr << "<p>" << std::endl;
	for( int i = 0; i < 20; ++i ) {
	    for( int i = 0; i < 12; ++i ) {
		ostr << "&nbsp;";
	    }
	    ostr << std::endl;
	}
	ostr << "</p>" << std::endl;
	return ostr;
    };
};

extern emptyParaHackType emptyParaHack;

/** \brief Populate options based on environment variables and stdin
 */
boost::program_options::basic_parsed_options<char>
parse_cgi_options( const boost::program_options::options_description& descr,
		   std::map<std::string,std::string>& query );

#endif
