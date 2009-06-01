#ifndef guardwebserve
#define guardwebserve

#include <iostream>
#include <iomanip>
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

/** \file webserve.hh 

	This defines wrappers around common CGI functionalities.

	See http://hoohoo.ncsa.uiuc.edu/cgi/env.html for documentation.
*/


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
		ptime::date_type today = now.date();        // Get the date part out of the time 
		ptime::date_type expires = today + v.lifetime;

		ostr << "Set-Cookie:" << v.name << "=" << v.value << ';'
			 << " Path=/;";
		if( v.lifetime != boost::posix_time::ptime::date_duration_type(0) ) {
			 ostr << " expires=" 
				  << expires.day_of_week()
				  << ", " << expires.day()
				  << ' ' << expires.month()
				  << " " << std::setfill('0') << std::setw(2) << expires.year()
				  << " " << std::setfill('0') << std::setw(2) << now.time_of_day().hours()
				  << ':' << std::setfill('0') << std::setw(2) << now.time_of_day().minutes()
				  << ':' << std::setfill('0') << std::setw(2) << now.time_of_day().seconds()
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
