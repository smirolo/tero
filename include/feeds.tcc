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

template<typename postFilter>
void feedContent<postFilter>::fetch( session& s, 
				    const boost::filesystem::path& pathname )
{
    using namespace boost::filesystem;

    path dirname(s.abspath(is_directory(pathname) ?
			   pathname : pathname.parent_path()));
    mailParser::fetch(s,dirname);
}


template<typename postFilter>
void feedNames<postFilter>::fetch( session& s, 
				   const boost::filesystem::path& pathname )
{
    using namespace boost::filesystem;

    path dirname(s.abspath(is_directory(pathname) ?
			   pathname : pathname.parent_path()));

    for( directory_iterator entry = directory_iterator(dirname); 
	 entry != directory_iterator(); ++entry ) {
	boost::smatch m;
	if( !is_directory(*entry) 
	    && boost::regex_match(entry->string(),m,filematch) ) {	
	    path filename(dirname / entry->filename());
	    std::string reluri = s.subdirpart(s.valueOf("siteTop"),
					      filename).string();	
	    post p;
	    p.guid = reluri;
	    p.title = reluri;
	    
	    std::stringstream s;
	    writelink(s,reluri);
	    p.descr = s.str();
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
		    } else {
			p.authorEmail = pwd->pw_name;
		    }
		}
#if 1
		/* \todo remove this code once we figure out how to get
		         the domain name. */
		p.authorEmail = "info@ocalhost.localdomain";
#endif
	    }
	    writer.filters(p);
	}
    }
}


template<typename postFilter>
void feedRepository<postFilter>::fetch( session& s, 
			    const boost::filesystem::path& pathname ) 
{
    revisionsys *rev = revisionsys::findRev(s,pathname);
    if( rev ) {
	history hist;
	rev->checkins(hist,s,pathname);

	for( history::checkinSet::const_iterator ci = hist.checkins.begin(); 
	     ci != hist.checkins.end(); ++ci ) {
	    writer.filters(*ci);
	}	
    }
}

#if 0
/* deprecated - left here to check output of htmlwriter matches. */
void 
changedescr::fetch( session& s, const boost::filesystem::path& pathname )
{
    using namespace std;

    revisionsys *rev = revisionsys::findRev(s,pathname);
    if( rev ) {
	history hist;
	rev->checkins(hist,s,pathname);

	htmlEscaper esc;

#if 1
	for( history::checkinSet::const_iterator ci = hist.checkins.begin(); 
	     ci != hist.checkins.end(); ++ci ) {
	    *ostr << html::h(2) << ci->title << html::h(2).end();
	    *ostr << html::p();
	    *ostr <<  ci->time << " - " << ci->authorEmail;
	    *ostr << html::p::end;
	    *ostr << html::p();
	    esc.attach(*ostr);
	    *ostr << ci->descr;
	    esc.detach();
	    *ostr << html::p::end;
	    *ostr << html::p();
	    for( checkin::fileSet::const_iterator file = ci->files.begin(); 
		 file != ci->files.end(); ++file ) {
		*ostr << html::a().href(file->string()) << *file << html::a::end << "<br />" << std::endl;
	    }
	    *ostr << html::p::end;
	}
#else
	htmlwriter liner(*ostr);
	for( history::checkinSet::const_iterator ci = hist.checkins.begin(); 
	     ci != hist.checkins.end(); ++ci ) {
	    liner.filters(*ci);
	}	
#endif	  
    }
}
#endif

