/* Copyright (c) 2009-2013, Fortylines LLC
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

#include <time.h>
#include <utime.h>
#include <sys/stat.h>
#include <sstream>
#include <boost/regex.hpp>
#include "webserve.hh"
#include <uriparser/Uri.h>

/** related to CGI

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/

namespace tero {

boost::posix_time::ptime
getmtime( const boost::filesystem::path& pathname )
{
    using namespace boost::posix_time;

    struct stat s; 
    stat(pathname.string().c_str(), &s);
    return from_time_t(s.st_mtime);       /* seconds since the epoch */
}


std::string uriEncode( const std::string& s ) {
    char escRequest[s.size() * 9];
    char *last = uriEscapeExA(s.c_str(),&s.c_str()[s.size()],escRequest,
			      false/*spaceToPlus*/,false/*normalizeBreaks*/);
    *(last + 1) = '\0';
    return std::string(escRequest);
}


httpHeaderSet::httpHeaderSet()
    : firstTime(true), contentLengthValue(0), statusCode(0)
{
}


httpHeaderSet& httpHeaderSet::contentDisposition( const std::string& value, 
						  const std::string& filename )
{
    contentDispositionValue = value;
    contentDispositionFilename = filename;
    return *this;    
}


httpHeaderSet& httpHeaderSet::contentType( const std::string& value, 
					   const std::string& charset ) 
{
    contentTypeValue = value;
    contentTypeCharset = charset;
    return *this;
}

httpHeaderSet& httpHeaderSet::contentLength( size_t length ) 
{
    contentLengthValue = length;
    return *this;
}

httpHeaderSet& httpHeaderSet::location( const url& v ) {
    locationValue = v;
    return *this;
}


httpHeaderSet& httpHeaderSet::refresh( size_t delay, const url& v ) {
    refreshDelay = delay;
    refreshUrl = v;
    return *this;
}


httpHeaderSet& 
httpHeaderSet::setCookie( const std::string& name, 
			  const std::string& value, 
			  boost::posix_time::ptime::date_duration_type exp )
{
    setCookieName = name;
    setCookieValue = value;
    setCookieLifetime = exp;
    return *this;
}


httpHeaderSet& httpHeaderSet::status( unsigned int s )
{
    statusCode = s;
    return *this;
}


httpHeaderSet httpHeaders;

emptyParaHackType emptyParaHack;

url::url( const std::string& name ) {
#if 1
    port = 0;
	if( name.substr(0,7) == "file://" ) {
		protocol = "file";
		pathname = name.substr(7);
	} else {
    boost::smatch m;
    boost::regex e("(\\S+:)?(//[^/]+)?(:[0-9]+)?(\\S+)?",
		   boost::regex::normal | boost::regbase::icase);
    if( regex_search(name,m,e) ) {
		if( !m.str(1).empty() ) { 
			protocol = m.str(1).substr(0, m.str(1).size() - 1);
		}
		if( !m.str(2).empty() ) host = m.str(2).substr(2);
		port = atoi(m.str(3).c_str());
		if( m.str(4).size() > 2 && m.str(4)[0] == '/' &&  m.str(4)[1] == '/' ) {
			pathname = m.str(4).substr(2);
		} else {
			pathname = m.str(4);
		}
    }
	}
#else
	uriUriA uri;
	uriParserStateA state;
	state.uri = &uri;
	uriParseUriA(&state,name.begin(),name.end());
	
	protocol = std::string(uri.scheme.first,uri.scheme.afterLast);
	host = std::string(uri.hostText.first,uri.hostText.afterLast);
	port = atoi(uri.portText,uri.portText.afterLast);
	for( uriPathSegmentA *part = uri.pathHead; 
		 part != uri.pathTail; part = part->next ) {
		pathname = pathname / std::string(part->text.first,part->text.afterLast);
	}
	uriFreeUriMembersA(uri);
#endif
}


bool url::absolute() const {
	return !protocol.empty();
}


std::string url::string() const {
    std::stringstream s;
    s << *this;
    return s.str();
}


url url::operator/( const boost::filesystem::path& right ) const {
    return url(protocol,host,port,pathname / right);
}


int fromHexDigit( char c ) {
    if( (c >= 'A') & (c <= 'F') ) return ((int)c - 'A') + 10;
    if( (c >= 'a') & (c <= 'f') ) return ((int)c - 'a') + 10;
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

void parseCookieLine( std::map<std::string,std::string>& query,
		     const char* input, size_t len, 		     
		     const char sep = ';' ) {
    bool final = false;
    const char *last = &input[len];
    const char *nameFirst = input;
    while( nameFirst != last ) {
	const char *valueLast = last;
	const char *nameLast = strchr(nameFirst,'=');
	if( nameLast != NULL ) {
	    const char *valueFirst = nameLast + 1;
	    valueLast = strchr(valueFirst,sep);
	    if( valueLast == NULL ) {
		valueLast = last;
		final = true;
	    }
	    std::string key(nameFirst,std::distance(nameFirst,nameLast));
	    std::string value(valueFirst,std::distance(valueFirst,valueLast));
	    query[key] = value;
	}
	nameFirst = final ? valueLast : (valueLast + 1);
	// cookies (name=value) are separated by a semi-colon and a space
	while( *nameFirst == ' ' ) ++nameFirst;
    }
 
}

void parseCGILine( std::map<std::string,std::string>& query,
		     const char* input, size_t len, 		     
		     const char sep = '&' ) {
   int err = 0;
    int itemCount = 0;
    UriQueryListA *queryList = NULL;
    if( (err = uriDissectQueryMallocA(&queryList,&itemCount,
				      input,&input[len])) == URI_SUCCESS ) {	
	for( UriQueryListStructA *item = queryList;
	     item != NULL; item = item->next ) {
	    query[item->key] = item->value;
	}
	uriFreeQueryListA(queryList);
    }    
}


boost::program_options::basic_parsed_options<char> 
basic_cgi_parser::run() {
    using namespace boost::program_options;

#if 0
    extern char **environ;
    for( char **env = environ; *env != NULL; ++env ) {
		std::cerr << "[env] " << *env << std::endl;
    }
#endif    
    char *cookie = getenv("HTTP_COOKIE");
    if( cookie != NULL ) {
	parseCookieLine(query,cookie,strlen(cookie));
    }
    
    char *cPathInfo = getenv("PATH_INFO");
    if( cPathInfo != NULL ) {
	std::string pathInfo(cPathInfo);
        const std::string& command = positionalDesc->name_for_position(0);
	query[command] = pathInfo;
    }
	
    char *cQueryString = getenv("QUERY_STRING");
    if( cQueryString != NULL ) {
	parseCGILine(query,cQueryString,strlen(cQueryString));
    }
        
    long len;
    char *lenstr = getenv("CONTENT_LENGTH");
    if( lenstr != NULL && sscanf(lenstr,"%ld",&len) == 1 ) {
	char *buffer;
	char input[len];
	buffer = fgets(input, len + 1, stdin);
	parseCGILine(query,buffer,len);
    }

    /* initialize matching options */
    std::vector<option> opts;
    for( std::vector< boost::shared_ptr<option_description> >::const_iterator 
	     od = optDesc->options().begin(); od != optDesc->options().end(); 
	 ++od ) {	
	option opt;
#if 0
	opt.string_key = od->long_name();
	opt.value.push_back(query[od->long_name()]);
	opt.original_tokens.push_back(query[od->long_name()]);
	opts.push_back(opt);
#endif
    } 

    parsed_options result(optDesc);
    result.options = opts;
    
    // Presense of parsed_options -> wparsed_options conversion
    // does the trick.
    return basic_parsed_options<char>(result);
}


void pathSeg( const char** first, const char **segAfterLast, 
			  const char *afterLast ) {
	const char *p = *first;
	const char *last = p;

 dirsep:
	switch( *p ) {
	case '/':
		++p;
		if( p != afterLast ) goto dirsep;
	case '.':
		++p;
		if( p != afterLast ) goto selfref;			
	default:
		last = p;
		while( last != afterLast && *last != '/' ) ++last;			
	}
	goto done;
	
 selfref:
	if( *p == '/' ) {
		++p;
		if( p != afterLast ) goto dirsep;
	}
	last = p;
	while( last != afterLast && *last != '/' ) ++last;
	goto done;
	
 done:
	*first = p;
	*segAfterLast = last;
}

}
