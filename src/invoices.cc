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


void statement::contracts( const boost::filesystem::path& db ) {
    static const boost::regex head("(\\S+):(\\d+):(.+)");
 
    boost::filesystem::ifstream file;
    open(file,db);    

    timesMap::iterator curContract;
    while( !file.eof() ) {
	boost::smatch m;
	std::string line;
	getline(file,line);
	if( line.empty() ) {
	    curContract = billed.end();
	} else {
	    if( boost::regex_search(line,m,head) ) {
		curContract = billed.find(m.str(1));
		if( curContract != billed.end() ) {
		    curContract->second.tarif = atoi(m.str(2).c_str());
		    curContract->second.descr = m.str(3);
		}
	    } else {
		if( curContract != billed.end() ) {
		    curContract->second.address += line;
		    curContract->second.address += "\n";
		}
	    }
	}
    }

    file.close();
}


void statement::footer()
{
    using namespace boost::posix_time;

    for( timesMap::const_iterator tm = billed.begin();
	 tm != billed.end(); ++tm ) {

	std::cout << "<section>";
	std::cout << "<informaltable frame=\"none\">" << std::endl
		  << html::tr()
		  << "<td>"
		  << "<para>for services provided to:" << std::endl
		  <<  "<literallayout>";
	if( tm->second.address.empty() ) {
	    std::cout << tm->first;
	} else {
	    std::cout << tm->second.address;
	}
	std::cout << "</literallayout>" 
		  << "</para>" << std::endl
		  << "</td>"
		  << "<td align=\"right\">"
		  << "<date>"
		  << to_simple_string(tm->second.hours.begin()->second.date().end_of_month())
		  << "</date>"
		  << html::td::end << html::tr::end << "</informaltable>"
		  << std::endl;

	std::cout << "<section>"	    
		  << "<title>" << tm->second.descr << "</title>";
	std::cout << "<informaltable>"		  
		  << "<tgroup cols=\"3\" rowsep=\"0\" colsep=\"1\">"
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
	for( hourSet::const_iterator hr = tm->second.hours.begin(); 
	     hr != tm->second.hours.end(); ++hr ) {
	    ptime start = hr->first;
	    ptime stop = hr->second;
	    time_duration d = hours((stop - start).hours())
		+ (( (stop - start).minutes() > 0 ) ? hours(1) : hours(0));

	    size_t fee = d.hours() * tm->second.tarif;
	    total += fee;
#if 0
	    std::cout << html::tr()
		      << html::td() << start.date() << html::td::end
		      << "<td align=\"right\">" << d.hours() << html::td::end
		      << "<td align=\"right\">" << '$' << fee << html::td::end
		      << html::tr::end;
#else
	    std::cout << "<row>"
		      << "<entry>" << start.date() << "</entry>"
		      << "<entry>" << d.hours() << "</entry>"
		      << "<entry>$" << fee << "</entry>"
		      << "</row>";

#endif
	}

	for( size_t i = 20; i > tm->second.hours.size(); --i ) {
	    std::cout << "<row>"
		      << "<entry></entry>"
		      << "<entry></entry>"
		      << "<entry></entry>"
		      << "</row>";
	}
	std::cout << "</tbody>";
        std::cout << "<tfoot>";
	std::cout << "<row>" << "<entry>" << "Total" << "</entry>"
		  << "<entry>" << "</entry>"
		  << "<entry>$" << total << "</entry>"
		  << "</row>";
	
	std::cout << "</tfoot>"
		  << "</tgroup>"		 
		  << "</informaltable>"
		  << std::endl;

	std::cout << "</section></section>";
    }
}


void statement::timeRange( const std::string& msg,
			   const boost::posix_time::ptime start,
			   const boost::posix_time::ptime stop ) {

    billed[msg].hours.push_back(std::make_pair(start,stop));
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
		timeRange(client,start,stop);
	    }
	} catch(...) {
	    /* It is ok if we cannot interpret some lines in the log;
	       most notably the last empty line. */
	    msg = line;
	}
    }
    file.close();
    /* Load information associated to the contracts */
    contracts(s.valueOf("contractDb"));
    footer();
}
