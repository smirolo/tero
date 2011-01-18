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

template<typename cmp>
void feedOrdered<cmp>::provide()
{
    cmp c;
    std::sort(indices.begin(),indices.end(),c);
    first = indices.begin();
    last = indices.end();
    super::provide();
}



template<typename feedReader, typename postWriter>
void feedWriter<feedReader,postWriter>::fetch( session& s, 
			   const boost::filesystem::path& pathname ) const
{
    postWriter writer(s.out());
    feedReader feeds(pathname,0,feedIndex::maxLength);
    for( typename feedReader::iterator first = feeds.begin();
	 first != feeds.end(); ++first ) {
	if( first->descr.empty() ) {
	    std::stringstream strm;
	    const document* doc 
		= dispatchDoc::instance->select("document",first->guid);
	    if( doc != NULL ) {		
		session sout("view","semillaId",strm);
		doc->fetch(sout,first->guid);
		if( !sout.valueOf("title").empty() ) {
		    first->title = sout.valueOf("title");
		}
	    } else {		
		strm << "updated file ";
		writelink(strm,first->guid);
		first->descr = strm.str();	 
	    }
	    first->descr = strm.str();
	}
	writer.filters(*first);
    }

}


template<typename feedReader, typename postWriter>
void feedAggregate<feedReader,postWriter>::meta( session& s, 
			   const boost::filesystem::path& pathname ) const
{
    using namespace boost::filesystem;

    path p(s.abspath(pathname));   	
    path dirname(pathname.parent_path());
    path track(pathname.filename());

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


template<typename feedReader, typename postWriter>
void feedContent<feedReader,postWriter>::meta( session& s, 
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
	    const document* doc 
		= dispatchDoc::instance->select("document",reluri);
	    if( doc != NULL ) {		
		doc->meta(s,reluri);
	    } else {
		post p;
		p.title = reluri;	    
		p.guid = reluri;
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
}


template<typename postWriter>
void feedRepository<postWriter>::meta( session& s, 
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


