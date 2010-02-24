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

#include "invoices.hh"
#include "markup.hh"

void statement::header() {
    std::cout << "<?xml version=\"1.0\"?>" << std::endl
	      << "<!DOCTYPE chapter PUBLIC \"-//OASIS//DTD DocBook XML V4.5//EN\" \"http://www.oasis-open.org/docbook/xml/4.5/docbookx.dtd\">" << std::endl
	      << "<article id=\"invoice\">" << std::endl
	      << "<info>" << std::endl
	      << "<title>Invoice</title>" << std::endl
	      << "<author>Sebastien Mirolo</author>" << std::endl
	      << "<!-- <date>$Date: 2009/04/05 17:29:02 $</date> -->" << std::endl
	      << "</info>" << std::endl
	      << "<section>" << std::endl
	      << "<imageobject>" << std::endl
	      << "<imagedata fileref=\"logo.png\" format=\"PNG\"/>" << std::endl
	      << "</imageobject>"
	      << "<para>Fortylines Solutions</para>"
	      << "<para>22 Vandewater St. #201,</para>"
	      << "<para>San Francisco,</para>"
	      << "<para>CA 94133</para>"
	      << "<para>info@fortylines.com</para>"
	      << "<para>(415) 613 0793</para>"
	      << "<para>"
	      << "Date:"
	      << "</para>"
	      << "<para>"
	      << "Rovi Corporation"
	      << "..."
	      << "</para>"
	      << "<table rowsep=\"0\">"
	      << "<tgroup cols=\"4\">"
	      << "<thead>"
	      << "<row>"
	      << "<entry>Date</entry>"
	      << "<entry>Hours</entry>"
	      << "<entry>Fee</entry>"
	      << "</row>"
	      << "</thead>"
	      << "<tbody>" << std::endl;
}


void statement::footer()
{
    std::cout << "</tbody>"
	      << "</tgroup>"
	      << "</table>"
	      << "</section>"
	      << "</article>" << std::endl;

}


void statement::timeRange( const boost::posix_time::ptime start,
			   const boost::posix_time::ptime stop ) {
    using namespace boost::posix_time;
    time_duration d = hours((stop - start).hours())
	+ (( (stop - start).minutes() > 0 ) ? hours(1) : hours(0));
    
    std::cout << html::tr()
	      << html::td() << start.date() << html::td::end
	      << html::td() << d.hours() << html::td::end
	      << html::td() << (d.hours() * 150) << html::td::end
	      << html::tr::end;
}


void statement::fetch( session& s, const boost::filesystem::path& pathname ) {
    using namespace boost::system;
    using namespace boost::posix_time;
    using namespace boost::filesystem;

    std::string client = s.valueOf("client");
    ptime firstDay = time_from_string(s.valueOf("month"));
    ptime lastDay = firstDay + month(1);

    ifstream file(s.contributorLog());
    if( file.fail() ) {
	boost::throw_exception(basic_filesystem_error<path>(
				 std::string("error opening file"),
				 s.contributorLog(), 
				 error_code()));
    }

    header();
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

	    if( !prev.date().is_not_a_date() ) {
		timeRange(start,stop);
	    }
	    prev = start;
	} catch(...) {
	    /* It is ok if we cannot interpret some lines in the log;
	       most notably the last empty line. */
	}
    }
    file.close();
    footer();
}
