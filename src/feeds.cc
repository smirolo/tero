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

#include "feeds.hh"
#include "project.hh"

char allFilesPat[] = ".*";

#if 0
/* \todo this is an embed distinct filter */
void feedIndex::flush()
{
    provide();
    if( next ) {
        bool firstTime = true;
        const_iterator prev;
        for( const_iterator p = first; p != last  ; ++p ) {
            if( firstTime || prev->guid != p->guid ) {
                next->filters(*p);
                firstTime = false;
                prev = p;
            }
        }
        next->flush();
    }
}
#endif

void feedCompact::filters( const post& p )
{
    if( !prevInit || p.guid != prev ) {
        super::next->filters(p);
        prevInit = true;
        prev = p.guid;
    }
}



void summarize::filters( const post& v )
{
    if( next ) {
        post p = v;
        p.content = p.content.substr(0,std::min(length,p.content.size()));
        next->filters(p);
    }
}


void oneliner::filters( const post& p ) {
    *ostr << html::tr()
          << html::td() << p.time.date() << html::td::end
          << html::td() << p.author << html::td::end
          << html::td() << html::a().href(p.guid)
          << p.title
          << html::a::end << html::td::end;
    *ostr << html::td()
          << p.score
          << html::td::end;
    *ostr << html::tr::end;
}


void byTimeHtml::filters( const post& p ) {
    using namespace boost::gregorian;
    using namespace boost::posix_time;

    time_facet* facet(new time_facet(pubDate::shortFormat));
    (*ostr).imbue(std::locale((*ostr).getloc(), facet));

    if( prev_header.is_not_a_date_time() ) {
        *ostr << html::tr().classref("bytime-date")
              << html::td().colspan("3")
              << "Complete by " << p.time << html::td::end
              << html::tr::end;
    } else if( prev_header != p.time ) {
        boost::posix_time::ptime iter = prev_header;
        do {
            iter = iter + days(1);
            *ostr << html::tr().classref("bytime-date")
                  << html::td().colspan("3")
                  << "Complete by " << iter << html::td::end
                  << html::tr::end;
        } while( iter != p.time );
    }
    *ostr << html::tr()
          << html::td() << p.score << html::td::end
          << html::td() << p.author->name << html::td::end
          << html::td() << html::a().href(p.guid)
          << p.title
          << html::a::end << html::td::end
          << html::tr::end;
    prev_header = p.time;
}

