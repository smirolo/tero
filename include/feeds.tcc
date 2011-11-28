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

#include <boost/regex.hpp> 
#include <sys/stat.h>
#include <pwd.h>
#include "markup.hh"


template<typename cmp>
void feedOrdered<cmp>::provide()
{
    cmp c;
    super::provide();
    std::sort(first,last,c);   
    iterator p = first;
}


template<typename cmp>
void feedSelect<cmp>::filters( const post& p )
{
    cmp c;
    typename cmp::keyType k = c.key(p);
    if( matchKeys.find(k) != matchKeys.end() ) {
        super::next->filters(p);
    }
}


template<typename feedBase>
const typename feedPage<feedBase>::difference_type 
feedPage<feedBase>::maxLength = ~((int)1 << ((sizeof(int) << 3) - 1));


template<typename feedBase>
void feedPage<feedBase>::provide() {
    super::provide();
    typename super::postSet::iterator second = super::first;   
    if( std::distance(second,super::last) >= base ) {
	std::advance(super::first,base);
    }
    second = super::first;
    if( std::distance(second,super::last) >= length ) {
	super::last = super::first;
	std::advance(super::last,length);	
    }
}


template<typename defaultWriter, const char* varname, const char *filePat>
void feedAggregate( session& s, 
		    const boost::filesystem::path& pathname )
{
    using namespace boost::filesystem;

    defaultWriter writer(s.out());
    if( !s.feeds ) {
		s.feeds = &writer;
    }
    path dirname(pathname.parent_path());
    path track(pathname.filename());

    bool subdirs = false;
    for( directory_iterator entry = directory_iterator(dirname); 
		 entry != directory_iterator(); ++entry ) {
		/* \todo include/exclude filtering should surely be done here. */
		if( is_directory(*entry) ) {	
			subdirs = true;
			url trackname(s.asUrl(*entry / track));
			dispatchDoc::instance()->fetch(s,varname,trackname);	    
		}
    }
#if 0
    /* \todo fix tokenizers */
    if( !subdirs ) {
		/* \todo find out how to handle commit posts and blog posts
		   without duplicates, picking the appropriate one. */
		feedContent<defaultWriter,filePat>(s,dirname);
    }
#endif
    if( s.feeds == &writer ) {
		s.feeds->flush();
		s.feeds = NULL;
    }
}

/* \todo see next HACK! */
void blogEntryFetch( session& s, const boost::filesystem::path& pathname );


template<typename defaultWriter, const char* filePat>
void feedContent( session& s, const boost::filesystem::path& pathname ) {
    using namespace boost::filesystem;

    defaultWriter writer(s.out());
    if( !s.feeds ) {
	s.feeds = &writer;
    }

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
			path filename(*entry);
			url link(s.asUrl(filename));

			if( is_regular_file(filename) 
				&& boost::regex_search(filename.string(),
									   m,boost::regex(filePat)) ) {
				/* We build-up a post from the different callbacks 
				   that would be called to make up an regular html 
				   page through a composer. In case there is no default 
				   clause, we will pick-up which either information we 
				   can from the filesystem. */
				const fetchEntry* e 
					= dispatchDoc::instance()->select("document",filename.string());
				if( e->callback == blogEntryFetch ) {
					/* \todo HACK! to insures *tags* are set correctly
					   and non blog files are generated as posts. */
					dispatchDoc::instance()->fetch(s,"document",link);
				} else {	       
					std::stringstream content;
					std::ostream& prevDisp = s.out(content);
					post p;
					content.str("");
					if( !dispatchDoc::instance()->fetch(s,"title",link)) {
						content << s.asUrl(filename);
					}
					p.title = content.str();
					content.str("");
					if( !dispatchDoc::instance()->fetch(s,"author",link)) {
						metaFileOwner(s,filename);
					}
					p.author = content.str();
					p.authorEmail = authorEmail.value(s);
					content.str("");
					if( !dispatchDoc::instance()->fetch(s,"date",link)) {
						metaLastTime(s,filename);
					}
					try {
						p.time = from_mbox_string(content.str());
					} catch( std::exception& e ) {
						std::cerr << "unable to reconstruct time from \"" << content.str() << '"' << std::endl;
					}
					content.str("");
					/* \todo Be careful here, if there are no *.blog pattern
					   but there is a /blog/.* pattern in the dispatch table,
					   this will create an infinite loop that only stops when
					   the system runs out of file descriptor. I am not sure
					   how to avoid or pop an error for this case yet. */
					if( !dispatchDoc::instance()->fetch(s,"document",link)) {
						content << s.asUrl(filename);
					}    
					p.content = content.str();		
					s.out(prevDisp);
					s.feeds->filters(p);
				}
			}
		}	
    }

    if( s.feeds == &writer ) {
	s.feeds->flush();
	s.feeds = NULL;
    }
}


template<typename defaultWriter, const char* varname>
void feedLatestPosts( session& s, 
		      const boost::filesystem::path& pathname )
{
    defaultWriter writer(s.out());
    feedOrdered<orderByTime<post> > latests(&writer);
    feedPage<feedOrdered<orderByTime<post> > > feeds(latests,5,0); 
    if( !s.feeds ) {	
		s.feeds = &feeds;
    }

    feedAggregate<defaultWriter,varname,allFilesPat>(s,pathname);

    if( s.feeds == &feeds ) {
		s.feeds->flush();
		s.feeds = NULL;
    }
}


template<const char*varname>
void htmlSiteAggregate( session& s, const boost::filesystem::path& pathname ) 
{
    feedLatestPosts<htmlwriter,varname>(s,siteTop.value(s) / "index.feed");
}


template<const char*varname>
void rssSiteAggregate( session& s, const boost::filesystem::path& pathname )
{
    absUrlDecorator d(pathname,s);
    d.attach(s.out());
    feedLatestPosts<rsswriter,varname>(s,siteTop.value(s) / pathname.leaf());
    d.detach();
}


template<typename defaultWriter, const char* filePat>
void feedSummary( session& s, const boost::filesystem::path& pathname ) {
    defaultWriter writer(s.out());
    summarize feeds(&writer);
    if( !s.feeds ) {
		s.feeds = &feeds;
    }
	
    feedContent<defaultWriter,filePat>(s,pathname);
	
    if( s.feeds == &feeds ) {
		s.feeds->flush();
		s.feeds = NULL;
    }
}


