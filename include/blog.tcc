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

#include "markup.hh" 


template<typename cmp>
void blogSplat<cmp>::filters( const post& v ) {
    post p = v;
    post::headersMap::const_iterator tags 
	= p.moreHeaders.find(cmp::name);
    if( tags != p.moreHeaders.end() ) {
	size_t first = 0, last = 0;
	while( last != tags->second.size() ) {
	    if( tags->second[last] == ',' ) {
		std::string s 
		    = strip(tags->second.substr(first,last-first));
		if( !s.empty() ) {
		    p.tag = s;
		    next->filters(p);
		}
		first = last + 1;
	    }
	    ++last;
	}
	std::string s 
	    = strip(tags->second.substr(first,last-first));
	if( !s.empty() ) {
	    p.tag = s;
	    next->filters(p);
	}
    } else {
	next->filters(p);
    }
}


template<typename cmp>
void blogInterval<cmp>::provide()
{
    cmp c;
    super::provide();

    /* sorted from decreasing order most recent to oldest. */
    super::first = std::lower_bound(super::first,super::last,bottom,c);
    if( super::first == super::posts.end() ) {
	super::first = super::posts.begin();
    }
    super::last = std::upper_bound(super::first,super::last,top,c);

#if 0
    std::cerr << "[blogInterval] provide:" << std::endl;
    for( typename super::const_iterator f = super::first; 
	 f != super::last; ++f ) {
	std::cerr << f->time << " " << f->title << std::endl;
    }
#endif
}


template<typename defaultWriter, const char* varname, const char* filePat, 
	 typename cmp>
void blogByInterval( session& s, const boost::filesystem::path& pathname )
{
    cmp c;
    defaultWriter writer(s.out());    
    typename cmp::valueType lower, upper;
    try {
	std::string firstName = boost::filesystem::basename(pathname);	
	lower = c.first(firstName);
	upper = c.last(firstName);
    } catch( std::exception& e ) {
	lower = c.first();
	upper = c.last();
    }
#if 0
    std::cerr << "[blogByInterval] from " << lower.time << " to " << upper.time
	      << std::endl;
#endif
    blogInterval<cmp> interval(&writer,lower,upper);
    blogSplat<cmp> feeds(&interval);
    if( !s.feeds ) {
	s.feeds = &feeds;
    }
    
    feedContent<defaultWriter,filePat>(s,pathname);

    if( s.feeds == &feeds ) {
	s.feeds->flush();
	s.feeds = NULL;
    }
}

template<const char* varname, const char* filePat>
void blogByIntervalDate( session& s, const boost::filesystem::path& pathname )
{
    blogByInterval<htmlwriter,varname,filePat,orderByTime<post> >(s,pathname);
}

template<const char* varname, const char* filePat>
void blogByIntervalTags( session& s, const boost::filesystem::path& pathname )
{
    blogByInterval<htmlwriter,varname,filePat,orderByTag<post> >(s,pathname);
}


template<typename cmp>
void bySet<cmp>::filters( const post& p ) 
{
    cmp c;
    typename cmp::keyType k = c.key(p);
    typename linkSet::iterator l = links.find(k);
    if( l != links.end() ) {
	++l->second;
    } else {
	links[k] = 1;
    }
}
   

template<typename cmp>
void bySet<cmp>::flush()
{
    using namespace boost::gregorian;
    using namespace boost::posix_time;

    /* We donot use pubDate::format here because the code logic relies
       on special formatted links to create subsequent pages. */
    time_facet* facet(new time_facet("%b %Y"));
    time_facet* linkfacet(new time_facet("%Y-%m-01"));    

    cmp c;
    std::stringstream strm;
    strm.imbue(std::locale(strm.getloc(), linkfacet));
    ostr->imbue(std::locale(ostr->getloc(), facet));

    /* Display keys and the associated number of blog entries */
    for( typename linkSet::const_iterator link = links.begin();
	 link != links.end(); ++link ) {
	strm.str("");
	strm << "/" << root << "/" << c.name << "/" << link->first;       
	*ostr << html::a().href(strm.str()) 
		<< link->first << " (" << link->second << ")"
		<< html::a::end << html::linebreak;
    }
}


template<typename cmp, const char *filePat>
void blogSetLinksFetch( session& s, const boost::filesystem::path& pathname )
{
    boost::filesystem::path blogroot 
	= s.root(s.abspath(pathname),"blog") / "blog";

    bySet<cmp> count(s.out(),s.subdirpart(siteTop.value(s),blogroot));
    blogSplat<cmp> feeds(&count);

    if( !s.feeds ) {
	s.feeds = &feeds;
    }

    /* workaround Ubuntu/gcc-4.4.3 is not happy with boost::regex(filePat) 
       passed as parameter to parser. */
    boost::regex pat(filePat);
    mailParser parser(pat,*s.feeds,false);
    parser.fetch(s,blogroot);

    if( s.feeds == &feeds ) {
	s.feeds->flush();
	s.feeds = NULL;
    }
}


template<const char *filePat>
void blogDateLinks( session& s, const boost::filesystem::path& pathname ) {
    blogSetLinksFetch<orderByTime<post>,filePat>(s,pathname);
}

template<const char *filePat>
void blogTagLinks( session& s, const boost::filesystem::path& pathname ) {
    blogSetLinksFetch<orderByTag<post>,filePat>(s,pathname);
}



