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

char allFilesPat[] = ".*";

feedIndex::indexSet feedIndex::indices;

feedIndex::iterator feedIndex::first;
feedIndex::iterator feedIndex::last;

void feedIndex::filters( const post& p ) {
    indices.push_back(p);
}


void feedIndex::provide() {
    first = indices.begin();
    last = indices.end();
}


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


postFilter *globalFeeds = NULL;


void feedWriterFetch( session& s, 
		      const boost::filesystem::path& pathname )
{
#if 0
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
		if( dispatchDoc::instance->fetch(s,"document",first->guid)) {
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
#endif
}


void feedRepositoryPopulate( session& s, 
			    const boost::filesystem::path& pathname )
{
    revisionsys *rev = revisionsys::findRev(s,pathname);
    if( rev ) {
	history hist;
	boost::filesystem::path base =  boost::filesystem::path("/") 
	    / s.subdirpart(s.valueOf("siteTop"),rev->rootpath);
	std::string projname = s.subdirpart(s.valueOf("srcTop"),rev->rootpath).string();
	if( projname[projname.size() - 1] == '/' ) {
	    projname = projname.substr(0,projname.size() - 1);
	}
	s.vars["title"] = projname;
	rev->checkins(hist,s,pathname);
	for( history::checkinSet::iterator ci = hist.checkins.begin(); 
	     ci != hist.checkins.end(); ++ci ) {
	    ci->normalize();
	    std::stringstream strm;
	    strm << html::p();
	    strm << projname << " &nbsp;&mdash;&nbsp; " << ci->guid << "<br />";
	    strm << ci->descr;
	    strm << html::p::end;
	    strm << html::pre();
	    for( checkin::fileSet::const_iterator file = ci->files.begin(); 
		 file != ci->files.end(); ++file ) {
		/* \todo link to diff with previous revision */
		writelink(strm,base,*file);
		strm << std::endl;
	    }
	    strm << html::pre::end;
	    post p(*ci);
	    p.descr = strm.str();
	    globalFeeds->filters(p);
	}	
    }
}


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

