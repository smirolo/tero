// -*- C++ -*-
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

template<typename compareFunc>
void feedIndex::provide( const compareFunc& cmp )
{
    std::sort(indices.begin(),indices.end(),cmp);
#if 0
    for( indexSet::const_iterator p = indices.begin();
	 p != indices.end(); ++p ) {
	std::cerr << p->time << " - "
		  << p->tag << " - " << p->title << std::endl;
    }
#endif
}


template<typename postFilter>
const size_t feedBlock<postFilter>::base = 0;

template<typename postFilter>
const int feedBlock<postFilter>::length = 10;


template<typename postFilter>
void feedBlock<postFilter>::fetch( session& s, const boost::filesystem::path& pathname ) const
{
    std::cerr << "!!! fetch block " << pathname << std::endl;
    postFilter writer(s.out());
    feedIndex::instance.provide(orderByTime<post>());
    write(feedIndex::instance.indices.begin(),
	  feedIndex::instance.indices.end(),writer);
}


template<typename postFilter>
void feedBlock<postFilter>::write( feedIndex::indexSet::const_iterator first,
				   feedIndex::indexSet::const_iterator last,
				   postFilter& writer ) const
{
    if( std::distance(first,last) >= base ) {
	std::advance(first,base);
    }
    feedIndex::indexSet::const_iterator second = first;
    if( std::distance(second,last) >= length ) {
	std::advance(second,length);
    } else {
	second = last;
    }
    super::write(first,second,writer);
}


template<typename postFilter>
void feedAggregate<postFilter>::meta( session& s, 
			   const boost::filesystem::path& pathname ) const
{
    using namespace boost::filesystem;

    path p(s.abspath(pathname));   	
    path dirname(pathname.parent_path());
    path track(pathname.filename());

    std::cerr << "!!! aggregate " << p << std::endl;

    for( directory_iterator entry = directory_iterator(dirname); 
	 entry != directory_iterator(); ++entry ) {
	boost::smatch m;
	if( is_directory(*entry) ) {	
	    path trackname(dirname / entry->filename() / track);
	    const document* doc 
		= dispatchDoc::instance->select("document",trackname.string());
	    if( doc != NULL ) {		
		doc->meta(s,trackname);
	    } else {
		meta(s,trackname);
	    }
	}
    }
}

template<typename postFilter>
void feedAggregate<postFilter>::fetch( session& s, const boost::filesystem::path& pathname ) const
{
    postFilter writer(s.out());
    write(feedIndex::instance.indices.begin(),
	      feedIndex::instance.indices.end(),
	      writer);
}

template<typename postFilter>
void feedContent<postFilter>::meta( session& s, 
				    const boost::filesystem::path& pathname ) const
{
    using namespace boost::filesystem;

    path dirname(s.abspath(is_directory(pathname) ?
			   pathname : pathname.parent_path()));
    std::cerr << "WTH? " << pathname << std::endl;
    mailParser m(filePat,feedIndex::instance,true);
    m.fetch(s,dirname);
}


template<typename postFilter>
void feedContent<postFilter>::fetch( session& s, 
				    const boost::filesystem::path& pathname ) const
{
    postFilter writer(s.out());
    write(feedIndex::instance.indices.begin(),
	      feedIndex::instance.indices.end(),
	      writer);
}


template<typename postFilter>
void feedNames<postFilter>::meta( session& s, 
				   const boost::filesystem::path& pathname ) const
{
    using namespace boost::filesystem;

    path dirname(s.abspath(is_directory(pathname) ?
			   pathname : pathname.parent_path()));

    for( directory_iterator entry = directory_iterator(dirname); 
	 entry != directory_iterator(); ++entry ) {
	boost::smatch m;
	
	if( !is_directory(*entry) 
	    && boost::regex_match(entry->string(),m,filePat) ) {	
	    path filename(dirname / entry->filename());	    
	    std::string reluri = s.subdirpart(s.valueOf("siteTop"),
					      filename).string();	
	    post p;
	    p.guid = reluri;
	    p.title = reluri;
	    
	    std::stringstream strm;
	    strm << "updated file ";
	    writelink(strm,reluri);
	    p.descr = strm.str();
	    std::time_t lwt = last_write_time(filename);
	    p.time = boost::posix_time::from_time_t(lwt);
	    
	    {
		/* \todo donot seem to find functionality to find
		         owner through boost. */
		struct stat statbuf;
		if( stat(filename.string().c_str(),&statbuf) ) {
		    p.authorEmail = "info";
		} else {
		    struct passwd *pwd = getpwuid(statbuf.st_uid);
		    if( pwd == NULL ) {
			p.authorEmail = "info";
			p.authorName = "anonymous";
		    } else {
			p.authorEmail = pwd->pw_name;
			p.authorName = pwd->pw_gecos;
		    }
		}
		p.authorEmail += std::string("@") + s.valueOf("domainName");
	    }
	    feedIndex::instance.filters(p);
	}
    }
}


template<typename postFilter>
void feedNames<postFilter>::fetch( session& s, 
				    const boost::filesystem::path& pathname ) const
{
    postFilter writer(s.out());
    write(feedIndex::instance.indices.begin(),
	  feedIndex::instance.indices.end(),
	  writer);
}


template<typename postFilter>
void feedRepository<postFilter>::meta( session& s, 
			    const boost::filesystem::path& pathname ) const
{
    revisionsys *rev = revisionsys::findRev(s,pathname);
    if( rev ) {
	history hist;
	s.vars["title"] = s.subdirpart(s.valueOf("srcTop"),rev->rootpath).string();
	rev->checkins(hist,s,pathname);
	for( history::checkinSet::iterator ci = hist.checkins.begin(); 
	     ci != hist.checkins.end(); ++ci ) {
	    ci->normalize();
	    std::stringstream strm;
	    strm << html::p();
	    strm << ci->guid << " &nbsp;&mdash;&nbsp; ";
	    strm << ci->descr;
	    strm << html::p::end;
	    strm << html::pre();
	    for( checkin::fileSet::const_iterator file = ci->files.begin(); 
		 file != ci->files.end(); ++file ) {
		/* \todo link to diff with previous revision */
		writelink(strm,*file);
		strm << std::endl;
	    }
	    strm << html::pre::end;
	    post p(*ci);
	    p.descr = strm.str();
	    feedIndex::instance.filters(p);
	}	
    }
}


template<typename postFilter>
void feedRepository<postFilter>::fetch( session& s, 
				    const boost::filesystem::path& pathname ) const
{
    postFilter writer(s.out());
    write(feedIndex::instance.indices.begin(),
	      feedIndex::instance.indices.end(),
	      writer);
}
