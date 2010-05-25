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
#include "calendar.hh"
#include "markup.hh"

void calendar::fetch( session& s, const boost::filesystem::path& pathname ) {
    using namespace std;
    using namespace boost::gregorian;
    using namespace boost::posix_time;

    date today;
    std::string ms = s.valueOf("month");
    if( ms.empty() ) {
	today = second_clock::local_time().date();
    } else {
	today = date(from_simple_string(ms));
    }

    date firstOfMonth(today.year(),today.month(),1);
    date lastOfMonth(firstOfMonth.end_of_month());

    static const char *weekdayNames[] = {
	"Sunday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday"
    };

    {
	std::stringstream s;
	s << "/schedule.ics?month=" << (firstOfMonth - days(1)) << "";	
	cout << html::a().href(s.str()) << "prev" << html::a::end;
	s.str("");
	cout << " ";
	s << "/schedule.ics?month=" << (lastOfMonth + days(1)) << "";	
	cout << html::a().href(s.str()) << "next" << html::a::end;	
    }    
    
    cout << html::table();
    cout << "<caption>" << today.month() << " " << today.year() 
	 << "</caption>" << std::endl;
    cout << html::tr();
    for( int day = 0; day < 7; ++day ) {
	cout << html::th()
	     << weekdayNames[day]
	     << html::th::end;
    }
    cout << html::tr::end;

    date firstOfCal = firstOfMonth - days((size_t)firstOfMonth.day_of_week());
    for( date d = firstOfCal; d < lastOfMonth; ) {
	cout << html::tr();
	for( int day = 0; day < 7; ++day ) {
	    cout << html::td()
		 << d
		 << html::td::end;	    
	    d += days(1);	    
	}
	cout << html::tr::end;
    }
    cout << html::table::end;
}
