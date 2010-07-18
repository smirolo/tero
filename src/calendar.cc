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


bool calendar::walkNodeEntry::operator<( const walkNodeEntry& right ) const {
    return strcmp(name,right.name) < 0;
}


calendar::walkNodeEntry calendar::walkers[] = {
    { "ACTION", &calendar::any, &calendar::any }, /* Property Name */
    { "ATTACH", &calendar::any, &calendar::any }, /* Property Name */
    { "ATTENDEE", &calendar::any, &calendar::any }, /* Property Name */
    { "CALSCALE", &calendar::any, &calendar::any }, /* Property Name */
    { "CATEGORIES", &calendar::any, &calendar::any }, /* Property Name */
    { "CLASS", &calendar::any, &calendar::any }, /* Property Name */
    { "COMMENT", &calendar::any, &calendar::any }, /* Property Name */
    { "COMPLETED", &calendar::any, &calendar::any }, /* Property Name */
    { "CONTACT", &calendar::any, &calendar::any }, /* Property Name */
    { "DESCRIPTION", &calendar::any, &calendar::any }, /* Property Name */
    { "DTEND", &calendar::any, &calendar::any }, /* Property Name */
    { "DTSTART", &calendar::any, &calendar::any }, /* Property Name */
    { "DUE", &calendar::any, &calendar::any }, /* Property Name */
    { "DURATION", &calendar::any, &calendar::any }, /* Property Name */
    { "EXDATE", &calendar::any, &calendar::any }, /* Property Name */
    { "GEO", &calendar::any, &calendar::any }, /* Property Name */
    { "FREEBUSY", &calendar::any, &calendar::any }, /* Property Name */
    { "LOCATION", &calendar::any, &calendar::any }, /* Property Name */
    { "METHOD", &calendar::any, &calendar::any }, /* Property Name */
    { "ORGANIZER", &calendar::any, &calendar::any }, /* Property Name */
    { "PERCENT-COMPLETE", &calendar::any, &calendar::any }, /* Property Name */
    { "PRIORITY", &calendar::any, &calendar::any }, /* Property Name */
    { "PRODID", &calendar::any, &calendar::any }, /* Property Name */
    { "RDATE", &calendar::any, &calendar::any }, /* Property Name */
    { "RECURRENCE-ID", &calendar::any, &calendar::any }, /* Property Name */
    { "RELATED-TO", &calendar::any, &calendar::any }, /* Property Name */
    { "RESOURCES", &calendar::any, &calendar::any }, /* Property Name */
    { "RRULE", &calendar::any, &calendar::any }, /* Property Name */
    { "STATUS", &calendar::any, &calendar::any }, /* Property Name */
    { "SUMMARY", &calendar::any, &calendar::any }, /* Property Name */
    { "TRANSP", &calendar::any, &calendar::any }, /* Property Name */
    { "TZID", &calendar::any, &calendar::any }, /* Property Name */
    { "TZNAME", &calendar::any, &calendar::any }, /* Property Name */
    { "TZOFFSETFROM", &calendar::any, &calendar::any }, /* Property Name */
    { "TZOFFSETTO", &calendar::any, &calendar::any }, /* Property Name */
    { "TZURL", &calendar::any, &calendar::any }, /* Property Name */
    { "UID", &calendar::any, &calendar::any }, /* Property Name */
    { "URL", &calendar::any, &calendar::any }, /* Property Name */
    { "VALARM", &calendar::any, &calendar::any }, /* Component Name */
    { "VCALENDAR", &calendar::any, &calendar::any },
    { "VERSION", &calendar::any, &calendar::any }, /* Property Name */
    { "VEVENT", &calendar::any, &calendar::any },
    { "VFREEBUSY", &calendar::any, &calendar::any },
    { "VTIMEZONE", &calendar::any, &calendar::any },
    { "VTODO", &calendar::any, &calendar::any },
    { "VJOURNAL", &calendar::any, &calendar::any }
};


void calendar::any( const std::string& s ) {
}


calendar::walkNodeEntry* calendar::walker( const std::string& s ) {
    walkNodeEntry *walkersEnd 
	= &walkers[sizeof(walkers)/sizeof(walkNodeEntry)];
    walkNodeEntry *d;
    walkNodeEntry key;
    
    key.name = s.c_str();
    d = std::lower_bound(walkers,walkersEnd,key);
    if( d != walkersEnd ) return d;
    return NULL;
}


void calendar::parse( session& s, std::istream& ins ) {

    static const boost::regex syntax("^(\\S+):(.+)");

    size_t lineCount = 0;
    while( !ins.eof() ) {
	boost::smatch m;
	std::string line;
	std::getline(ins,line);
	++lineCount;
	if( regex_match(line,m,syntax) ) {
	    walkNodeEntry *w = walker(m.str(1));
	    if( w ) (this->*(w->start))(m.str(2));
	}
    }
}


void calendar::fetch( session& s, const boost::filesystem::path& pathname ) {
    using namespace std;
    using namespace boost::gregorian;
    using namespace boost::posix_time;

    if( boost::filesystem::exists(pathname) ) {
	boost::filesystem::ifstream input;
	open(input,pathname);
	parse(s,input);
	input.close();
    }

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
