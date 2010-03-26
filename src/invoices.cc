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

#include <boost/date_time/gregorian/gregorian.hpp>
#include "invoices.hh"
#include "markup.hh"

void statement::header() {
}


void statement::footer()
{
    using namespace boost::posix_time;

    for( timesMap::const_iterator tm = billed.begin();
	 tm != billed.end(); ++tm ) {
#if 0
	boost::filesystem::ofstream file;
	open(file,pathname);
#endif
	std::cout << "<?xml version=\"1.0\"?>" << std::endl
		  << "<section xmlns=\"http://docbook.org/ns/docbook\""
		  << " xmlns:xlink=\"http://www.w3.org/1999/xlink\">"
		  << std::endl
	      << "<info>" << std::endl
	      << "<title>Invoice</title>" << std::endl
		  << "<author><personname><firstname>Sebastien</firstname><surname>Mirolo</surname></personname></author>" << std::endl;
	std::cout << "<date>"
		  << to_simple_string(tm->second.begin()->second.date().end_of_month())
		  << "</date>";

	std::cout << "</info>" << std::endl
	      
		  << "<informaltable>" << std::endl
		  << html::tr() << html::td()
	      << "<mediaobject>" << std::endl
	      << "<imageobject>" << std::endl
	      << "<imagedata fileref=\"logo.png\" width=\"128\" format=\"PNG\"/>" << std::endl
	      << "</imageobject>"
		  << "</mediaobject>"
		  << html::td::end << html::td()
	      << "<address>Fortylines Solutions"
	      << "<street>22 Vandewater St. #201</street>"
	      << "<city>San Francisco</city>"
	      << "<state>CA</state> <postcode>94133</postcode></address>"
	      << "<email>info@fortylines.com</email>"
	      << "<para>(415) 613 0793</para>"
		  << html::td::end << html::tr::end << "</informaltable>";
       
	std::cout << "<para>"
		  << "for services provided to "
		  << tm->first << std::endl
		  << "..."
		  << "</para>";

	std::cout << "<table>"
		  << "<title>hours</title>"
		  << "<tgroup cols=\"3\" colsep=\"1\">"
		  << "<colspec colnum=\"1\" colname=\"Date\" align=\"left\" />"
		  << "<colspec colnum=\"2\" colname=\"Hours\" align=\"right\" />"
		  << "<colspec colnum=\"3\" colname=\"Fee\" align=\"right\" />"
		  << "<thead>"
		  << "<row>"
		  << "<entry>Date</entry>"
		  << "<entry>Hours</entry>"
		  << "<entry>Fee</entry>"
		  << "</row>" 
		  << "</thead>"
		  << "<tbody>" << std::endl;

	size_t total = 0;
	for( hourSet::const_iterator hr = tm->second.begin(); 
	     hr != tm->second.end(); ++hr ) {
	    ptime start = hr->first;
	    ptime stop = hr->second;
	    time_duration d = hours((stop - start).hours())
		+ (( (stop - start).minutes() > 0 ) ? hours(1) : hours(0));

	    size_t fee = d.hours() * 150;
	    total += fee;
#if 0
	    std::cout << html::tr()
		      << html::td() << start.date() << html::td::end
		      << html::td() << d.hours() << html::td::end
		      << html::td() << fee << html::td::end
		      << html::tr::end;
#else
	    std::cout << "<row>"
		      << "<entry>" << start.date() << "</entry>"
		      << "<entry>" << d.hours() << "</entry>"
		      << "<entry>" << fee << "</entry>"
		      << "</row>";

#endif
	}
	std::cout << "</tbody>" 
		  << "</tgroup>"
		  << "</table>"
		  << std::endl;
	
	std::cout << "<informaltable>"
		  << "<tr>"
		  << "<th>" << "Total" << "</th>"
		  << "<th>" << total << html::th::end
		  << html::tr::end
		  << html::tr()
		  << html::th() << "Past dues" << html::th::end
		  << html::th() << 0 << html::th::end
		  << html::tr::end
		  << "</informaltable>";

	std::cout << "<para>"
		  << "Invoices are sent on the last day of the month."
		  << "Please disregard past dues if you have already mailed a check for them, thank you."
		  << "</para>" << std::endl;

	std::cout << "</section>"
		  << std::endl;
    }
}


void statement::timeRange( const std::string& msg,
			   const boost::posix_time::ptime start,
			   const boost::posix_time::ptime stop ) {

    billed[msg].push_back(std::make_pair(start,stop));
}


void statement::fetch( session& s, const boost::filesystem::path& pathname ) {
    using namespace boost::system;
    using namespace boost::gregorian;
    using namespace boost::posix_time;
    using namespace boost::filesystem;

    std::string client = s.valueOf("client");
    ptime firstDay(date(from_simple_string(s.valueOf("month"))));
    ptime lastDay(firstDay.date().end_of_month());

    ifstream file(s.contributorLog());
    if( file.fail() ) {
	boost::throw_exception(basic_filesystem_error<path>(
				 std::string("error opening file"),
				 s.contributorLog(), 
				 error_code()));
    }

    header();
    std::string msg;
    ptime start, stop;
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

	    if( firstDay < start && start < lastDay
		&& msg.compare(0,client.size(),client) == 0 ) {
		timeRange(msg,start,stop);
	    }
	} catch(...) {
	    /* It is ok if we cannot interpret some lines in the log;
	       most notably the last empty line. */
	    msg = line;
	}
    }
    file.close();
    footer();
}
