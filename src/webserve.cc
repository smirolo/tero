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

#include <time.h>
#include <utime.h>
#include <sys/stat.h>
#include <sstream>
#include <boost/regex.hpp>
#include "webserve.hh"

boost::posix_time::ptime
getmtime( const boost::filesystem::path& pathname )
{
    using namespace boost::posix_time;

    struct stat s; 
    stat(pathname.string().c_str(), &s);
    return from_time_t(s.st_mtime);       /* seconds since the epoch */
}


boost::filesystem::path 
relpath( const boost::filesystem::path& pathname,
	 const boost::filesystem::path& base ) {
    boost::filesystem::path::iterator first = pathname.begin();
    boost::filesystem::path::iterator second = base.begin();
    boost::filesystem::path result;
    
    for( ; first != pathname.end() & second != base.end(); 
		 ++first, ++second ) {
		if( *first != *second ) break;
    }
    for( ; first != pathname.end(); ++first ) {
		result /= *first;
    }
    return result;
}


htmlContentServe htmlContent;
emptyParaHackType emptyParaHack;

htmlContentServe::htmlContentServe() 
    : firstTime(true) {}


url::url( const std::string& name ) {
	boost::smatch m;
	boost::regex e("(\\S+:)?(//[^/]+)?(:[0-9]+)?(\\S+)?",
				   boost::regex::normal | boost::regbase::icase);
	port = 0;
	if( regex_search(name,m,e) ) {
		if( !m.str(1).empty() ) 
			protocol = m.str(1).substr(0, m.str(1).size() - 1);
		if( !m.str(2).empty() ) host = m.str(2).substr(2);
		port = atoi(m.str(3).c_str());
		pathname = m.str(4);
	}
}


bool url::absolute() const {
	return !protocol.empty();
}


std::string url::string() const {
    std::stringstream s;
    s << *this;
    return s.str();
}


int fromHexDigit( char c ) {
	if( c >= 'A' & c <= 'F' ) return ((int)c - 'A') + 10;
	if( c >= 'a' & c <= 'f' ) return ((int)c - 'a') + 10;
	return c - '0';
}


std::string reverseCGIFormatting( const std::string& str ) {
	using namespace std;

	stringstream result;
	const char *p = str.c_str();
	for( size_t i = 0; i < str.size(); ++i ) {
		if( p[i] == '%' ) {
			char escaped = static_cast<char>(fromHexDigit(p[i + 1]) * 16 
											 + fromHexDigit(p[i + 2]));
			if( escaped != '\r' ) result << escaped;
			i += 2;			
		} else if( p[i] == '+' ) {
			result << ' ';
		} else {
			result << p[i];
		}
	}
	return result.str();
}


void parse_cgi_line( std::vector<boost::program_options::option>& opts, 
		     const boost::program_options::options_description& descr,
		     const char* input, size_t len, const char sep = '&' ) {
    using namespace boost::program_options;
    
    bool final = false;
    const char *last = &input[len];
    const char *nameFirst = input;
    while( nameFirst != last ) {
	const char *valueLast = last;
	const char *nameLast = strchr(nameFirst,'=');
	if( nameLast != NULL ) {
	    option opt;
	    const char *valueFirst = nameLast + 1;
	    valueLast = strchr(valueFirst,sep);
	    if( valueLast == NULL ) {
		valueLast = last;
		final = true;
	    }
	    std::string key(nameFirst,std::distance(nameFirst,nameLast));
	    const option_description* optDescr = descr.find_nothrow(key,false);
	    if( optDescr != NULL ) {
		opt.string_key = key;
		opt.value.push_back(reverseCGIFormatting(std::string(valueFirst,
								     std::distance(valueFirst,valueLast))));
		opts.push_back(opt);
	    }
	}
	nameFirst = final ? valueLast : (valueLast + 1);
	// cookies (name=value) are separated by a semi-colon and a space
	while( *nameFirst == ' ' ) ++nameFirst;
    }
}


boost::program_options::basic_parsed_options<char>
parse_cgi_options( const boost::program_options::options_description& descr ) 
{
    using namespace boost::program_options;
    std::vector<option> opts;
    
    char *cookie = getenv("HTTP_COOKIE");
    if( cookie != NULL ) {
	parse_cgi_line(opts,descr,cookie,strlen(cookie),';');
    }
    
    char *cPathInfo = getenv("PATH_INFO");
    if( cPathInfo != NULL ) {
	std::string command("view");   // \todo hardcoded for wiki
	std::string pathInfo(cPathInfo);
	const option_description* optDescr = descr.find_nothrow(command,false);
	if( optDescr != NULL ) {
	    option opt;
	    opt.string_key = command;
	    opt.position_key = 1;
	    opt.value.push_back(pathInfo);
	    opt.original_tokens.push_back(pathInfo);
	    opts.push_back(opt);
	}
	
	char *cQueryString = getenv("QUERY_STRING");
	if( cQueryString != NULL ) {
	    parse_cgi_line(opts,descr,cQueryString,strlen(cQueryString));
	}
    }
    
    long len;
    char *lenstr = getenv("CONTENT_LENGTH");
    if( lenstr != NULL && sscanf(lenstr,"%ld",&len) == 1 ) {
	char input[len];
	fgets(input, len + 1, stdin);
	parse_cgi_line(opts,descr,input,len);
    }
    
    parsed_options result(&descr);
    result.options = opts;
    
    // Presense of parsed_options -> wparsed_options conversion
    // does the trick.
    return basic_parsed_options<char>(result);
}
