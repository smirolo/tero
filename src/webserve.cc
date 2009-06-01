#include <sstream>
#include <boost/regex.hpp>
#include "webserve.hh"

htmlContentServe htmlContent;

htmlContentServe::htmlContentServe() 
    : firstTime(true) {}


url::url( const std::string& name ) {
	boost::smatch m;
	boost::regex e("(\\S+:)?(//[^/]+)?(:[0-9]+)?(\\S+)?",
				   boost::regex::normal | boost::regbase::icase);
	port = 0;
	if( regex_search(name,m,e) ) {
		if( !m.str(1).empty() ) protocol = m.str(1).substr(0, m.str(1).size() - 1);
		if( !m.str(2).empty() ) host = m.str(2).substr(2);
		port = atoi(m.str(3).c_str());
		pathname = m.str(4);
	}
}


bool url::absolute() const {
	return !protocol.empty();
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
	for( int i = 0; i < str.size(); ++i ) {
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
			std::cerr << "QUERY_STRING=" << cQueryString << std::endl;
			parse_cgi_line(opts,descr,cQueryString,strlen(cQueryString));
		}
	}

	long len;
    char *lenstr = getenv("CONTENT_LENGTH");
    if( lenstr != NULL && sscanf(lenstr,"%ld",&len) == 1 ) {
		char input[len];
		fgets(input, len + 1, stdin);
		std::cerr << "STDIN=" << input << std::endl;
		parse_cgi_line(opts,descr,input,len);
	}

	parsed_options result(&descr);
	result.options = opts;
	
	// Presense of parsed_options -> wparsed_options conversion
	// does the trick.
	return basic_parsed_options<char>(result);
}
