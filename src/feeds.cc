/* Copyright (c) 2009-2011, Fortylines LLC
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


#if 0
void feedWriterFetch( session& s, 
		      const boost::filesystem::path& pathname )
{
    postWriter writer(s.out());

    bool firstTime = true;
    typename feedReader::iterator prev;
    std::stringstream strm;
    std::ostream& prevDisp = s.out(strm);

    for( typename feedReader::iterator first = feeds.begin();
	 first != feeds.end(); ++first ) {
	if( firstTime || first->guid != prev->guid ) {
	    if( first->descr.empty() ) {
		strm.str("");		
		if( dispatchDoc::instance()->fetch(s,"document",first->guid)) {
		    if( !s.valueOf("title").empty() ) {
			first->title = s.valueOf("title");
		    }
		} else {
		    strm << "updated file ";
		    writelink(strm,"",first->guid);
		    first->descr = strm.str();	 
		}
		first->descr = strm.str();
	    }

#if 0
	    /* undefined time. */
	    if( ) {
		std::time_t lwt = last_write_time(first->guid);
		first->time = boost::posix_time::from_time_t(lwt);
	    }
#endif	
	    /* \todo authorName is set from filename but not in mail.cc, only authorEmail there. 
	     What about commits? */
	    if( first->authorEmail.empty() ) {
		first->authorEmail = "info";
		first->authorName = "anonymous";
		/* \todo donot seem to find functionality to find
		   owner through boost. */
		struct stat statbuf;
		if( stat(first->guid.c_str(),&statbuf) == 0 ) {
		    struct passwd *pwd = getpwuid(statbuf.st_uid);
		    if( pwd != NULL ) {
			first->authorEmail = pwd->pw_name;
			first->authorName = pwd->pw_gecos;
		    }
		}
		first->authorEmail += std::string("@") + s.valueOf("domainName");
	    }
	    writer.filters(*first);
	}
	prev = first;
	firstTime = false;	
    }
    s.out(prevDisp);
}
#endif

#if 0
void htmlContentFetch( session& s, const boost::filesystem::path& pathname )
{
    typedef feedPage<feedOrdered<orderByTime<post> > > feedFilterType;
    htmlwriter writer(s.out());
    feedOrdered<orderByTime<post> > latests(&writer);
    feedFilterType feeds(latests,0,5); 

    postFilter *prev = globalFeeds;
    if( !globalFeeds ) {
	globalFeeds = &feeds;
    } 
    feedContentFetch<feedFilterType,allFilesPat>(s,pathname);
    globalFeeds = prev;
    if( !globalFeeds ) {
	feeds.flush();
    }
}


void rssContentFetch( session& s, const boost::filesystem::path& pathname )
{
    typedef feedPage<feedOrdered<orderByTime<post> > > feedFilterType;
    rsswriter writer(s.out());
    feedOrdered<orderByTime<post> > latests(&writer);
    feedFilterType feeds(latests,0,5); 

    postFilter *prev = globalFeeds;
    if( !globalFeeds ) {
	globalFeeds = &feeds;
    }
    feedContentFetch<feedFilterType,allFilesPat>(s,pathname);
    globalFeeds = prev;
    if( !globalFeeds ) {
	feeds.flush();
    }
}
#endif
