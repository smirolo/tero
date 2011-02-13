/* Copyright (c) 2011, Fortylines LLC
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

#include "changelist.hh"
#include <sys/stat.h>
#include <pwd.h>
#include "markup.hh"

template<typename cmp>
void feedOrdered<cmp>::provide()
{
    cmp c;
    super::provide();
    std::sort(first,last,c);   
}

template<typename feedBase>
const typename feedPage<feedBase>::difference_type 
feedPage<feedBase>::maxLength = ~((int)1 << ((sizeof(int) << 3) - 1));


template<typename feedBase>
void feedPage<feedBase>::provide() {
    super::provide();
    if( std::distance(super::first,super::last) >= base ) {
	std::advance(super::first,base);
    }
    typename super::indexSet::iterator second = super::first;
    if( std::distance(second,super::last) >= length ) {
	std::advance(second,length);
	super::last = second;
    }
}


template<typename postWriter>
void feedAggregate<postWriter>::aggregate( session& s, 
			   const boost::filesystem::path& pathname ) const
{
    using namespace boost::filesystem;

    path dirname(pathname.parent_path());
    path track(pathname.filename());

    std::stringstream nodisplay;
    std::ostream& prevDisp = s.out(nodisplay);
    for( directory_iterator entry = directory_iterator(dirname); 
	 entry != directory_iterator(); ++entry ) {
	boost::smatch m;
	if( is_directory(*entry) ) {	
	    path trackname(dirname / entry->filename() / track);
	    if( !dispatchDoc::instance->fetch(s,varname,trackname)) {
		aggregate(s,trackname);
	    }
	}
    }
    s.out(prevDisp);
}

template<typename postWriter>
void feedAggregate<postWriter>::fetch( session& s, 
			   const boost::filesystem::path& pathname ) const
{
    aggregate(s,pathname);
    super::feeds->flush();
}


template<typename postWriter>
void feedRepository<postWriter>::fetch( session& s, 
			    const boost::filesystem::path& pathname ) const
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
	    super::feeds->filters(p);
	}	
    }
}


template<typename postWriter>
feedLatestPosts<postWriter>::feedLatestPosts( const std::string& v ) 
    : feedAggregate<postWriter>(v)
{
}

template<typename postWriter>
void feedLatestPosts<postWriter>::fetch( session& s, 
			 const boost::filesystem::path& pathname ) const
{
    postFilter *prev = super::feeds;

    postWriter writer(s.out());
    /* \todo get a feedPage in-between */
    feedOrdered<orderByTime<post> > latests(&writer);
    feedPage<feedOrdered<orderByTime<post> > > feeds(latests,0,5); 
    super::feeds = &feeds;
    super::fetch(s,pathname);

    super::feeds = prev;
}

template<typename postWriter>
void  feedSummary<postWriter>::fetch( session& s, 
			    const boost::filesystem::path& pathname ) const
{
    summarize<postWriter> feeds(s.out());
    postFilter *prev = super::feeds;
    if( !super::feeds ) {
	super::feeds = &feeds;
    }
    super::fetch(s,pathname);
    super::feeds = prev;
}
