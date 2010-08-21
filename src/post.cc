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

#include "mail.hh"
#include "markup.hh"
#include <boost/date_time/date_facet.hpp>

void 
post::addSessionVars( boost::program_options::options_description& opts )
{
    using namespace boost::program_options;

    options_description postOptions("posts");
    postOptions.add_options()
	("title",value<std::string>(),"title")
	("author",value<std::string>(),"author")
	("descr",value<std::string>(),"descr");
    opts.add(postOptions);
}

void post::normalize() {
    title = ::normalize(title);
    authorEmail = ::normalize(authorEmail);
}


bool post::valid() const {
    /* If there are any whitespaces in the guid, Thunderbird 3 will
       hang verifying the rss feed... */
    for( size_t i = 0; i < guid.size(); ++i ) {
	if( isspace(guid[i]) ) {
	    return false;
	}
    }    
    return (!title.empty() & !authorEmail.empty() & !descr.empty());
}


void postFilter::filters( const post& p ) {
    if( next ) next->filters(p);
}

void postFilter::flush() {
    if( next ) next->flush();
}


void htmlwriter::filters( const post& p ) {
    htmlEscaper esc;

    if( postNum > 0 ) {
	*ostr << html::div().classref( (postNum % 2 == 0) ? 
				       "postEven" : "postOdd");
    }

#if 0
    if( !p.title.empty() ) {
	*ostr << html::h(1);
	esc.attach(*ostr);
	*ostr << p.title;
	esc.detach();
	*ostr << html::h(1).end();
    }
#endif

    *ostr << html::h(2);
    esc.attach(*ostr);
    *ostr << p.time << " - " << p.authorEmail;
    esc.detach();
    *ostr << html::h(2).end();    

    *ostr << html::p();
    esc.attach(*ostr);
    *ostr << p.descr;
    esc.detach();
    *ostr << html::p::end;

    if( postNum > 0 ) {
	*ostr << html::div::end;
    }
    ++postNum;
}


void blogwriter::filters( const post& p ) {
    using namespace boost::gregorian;
    using namespace boost::posix_time;

#if 0
    date_facet* facet(new date_facet("%a, %e %b %d %Y"));
    (*ostr).imbue(std::locale((*ostr).getloc(), facet));
#else
    time_facet* facet(new time_facet("%a, %e %b %Y %H:%M:%S %F%Q"));
    (*ostr).imbue(std::locale((*ostr).getloc(), facet));
#endif

#if 0
    *ostr << "Title: " << p.title << std::endl;
    *ostr << "Date: " << p.time << std::endl;
    *ostr << "Author: " << p.author << std::endl;
#else
    *ostr << "From " << p.authorEmail << std::endl;
    if( !p.title.empty() ) {
	*ostr << "Subject: " << p.title << std::endl;
    }
    *ostr << "Date: " << p.time << std::endl;
    *ostr << "From: " << p.authorEmail << std::endl;    
#endif
    *ostr << "Score: " << p.score << std::endl;
    *ostr << std::endl << std::endl;
    /* \todo avoid description starting with "From " */
    *ostr << p.descr << std::endl << std::endl;
}
