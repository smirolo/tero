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

template<typename postWriter, const char*varname>
void feedAggregateRecursive( session& s, 
			      const boost::filesystem::path& pathname )
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
		feedAggregateRecursive<postWriter,varname>(s,trackname);
	    }
	}
    }
    s.out(prevDisp);
}

template<typename postWriter, const char*varname>
void feedAggregateFetch( session& s, 
			   const boost::filesystem::path& pathname )
{
    feedAggregateRecursive<postWriter,varname>(s,pathname);
    globalFeeds->flush();
}


template<typename postWriter>
void feedRepository( session& s, 
			    const boost::filesystem::path& pathname )
{
    postFilter *prev = globalFeeds;
    postWriter feeds(s.out());    
    globalFeeds = &feeds;
    feedRepositoryPopulate(s,pathname);
    globalFeeds = prev;
}


template<typename postWriter, const char*varname>
void feedLatestPostsFetch( session& s, 
			 const boost::filesystem::path& pathname )
{
    postFilter *prev = globalFeeds;

    postWriter writer(s.out());
    /* \todo get a feedPage in-between */
    feedOrdered<orderByTime<post> > latests(&writer);
    feedPage<feedOrdered<orderByTime<post> > > feeds(latests,0,5); 
    globalFeeds = &feeds;
    feedAggregateFetch<postWriter,varname>(s,pathname);

    globalFeeds = prev;
}


template<const char*varname>
void htmlSiteAggregate( session& s, const boost::filesystem::path& pathname ) 
{
    std::cerr << "!!! Got here? (" << (pathname / "index.feed") 
	      << ")" << std::endl;
    feedLatestPostsFetch<htmlwriter,varname>(s,s.abspath("/index.feed"));
}

template<const char*varname>
void rssSiteAggregate( session& s, const boost::filesystem::path& pathname )
{
    feedLatestPostsFetch<rsswriter,varname>(s,pathname);    
}


template<typename postWriter>
void feedSummaryFetch( session& s, 
			    const boost::filesystem::path& pathname )
{
    summarize<postWriter> feeds(s.out());
    postFilter *prev = globalFeeds;
    if( !globalFeeds ) {
	globalFeeds = &feeds;
    }
    feedContentFetch<postWriter,allFilesPat>(s,pathname);
    globalFeeds = prev;
}


template<typename postWriter, const char* filePat >
void feedContentFetch( session& s, 
		       const boost::filesystem::path& pathname )
{
    using namespace boost::filesystem;

    /* Always generate the feed from a content directory even
       when a file is passed as *pathname* */
    path base(pathname);
    while( base.string().size() > siteTop.value(s).string().size()
	   && !is_directory(base) ) {
	base = base.parent_path();
    }
    if( !base.empty() ) {
	for( directory_iterator entry = directory_iterator(base); 
	     entry != directory_iterator(); ++entry ) {
	    boost::smatch m;
	    if( !is_directory(*entry) 
		&& boost::regex_match(entry->string(),m,
				      boost::regex(filePat)) ) {
		path filename(base / entry->filename());	    
		std::string reluri = s.subdirpart(siteTop.value(s),
						  filename).string();	    
		/* We should pop out a post for those docs that don't
		   even when they fetch (book vs. mail/blogentry). */
		if( !dispatchDoc::instance->fetch(s,"document",reluri)) {
		    post p;
		    p.title = reluri;	    
		    p.guid = reluri;
		    std::time_t lwt = last_write_time(filename);
		    p.time = boost::posix_time::from_time_t(lwt);
		    globalFeeds->filters(p);
		}
	    }
	}
   }
}
