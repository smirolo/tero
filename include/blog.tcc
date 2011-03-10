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
void blogInterval<cmp>::provide()
{
    super::provide();
 
    cmp c;
    std::string firstName = boost::filesystem::basename(pathname);

    typename cmp::valueType bottom = c.first(firstName);
    typename cmp::valueType top = c.last(firstName);

    /* sorted from decreasing order most recent to oldest. */
    super::first = std::lower_bound(super::first,super::last,bottom,c);
    if( super::first == super::indices.end() ) {
	super::first = super::indices.begin();
    }
    super::last = std::upper_bound(super::first,super::last,top,c);
}


template<typename cmp>
void blogByIntervalFetch( session& s, const boost::filesystem::path& pathname ) {
    htmlwriter writer(s.out());
    blogInterval<cmp> feeds(&writer,pathname);
    postFilter *prev = globalFeeds;
    globalFeeds = &feeds;
    feedContentFetch<cmp,allFilesPat>(s,pathname);
    globalFeeds->flush();
    globalFeeds = prev;
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
    boost::filesystem::path blogroot = s.root(s.abspath(pathname),"blog") / "blog";
    bySet<cmp> filter(s.out(),s.subdirpart(siteTop.value(s),blogroot));

    postFilter *prev = globalFeeds;
    globalFeeds = &filter;

    mailParser parser(boost::regex(filePat),*globalFeeds,false);
    parser.fetch(s,blogroot);
    globalFeeds->flush();

    globalFeeds = prev; 
}

template<const char *filePat>
void blogDateLinks( session& s, const boost::filesystem::path& pathname ) {
    blogSetLinksFetch<orderByTime<post>,filePat>(s,pathname);
}

template<const char *filePat>
void blogTagLinks( session& s, const boost::filesystem::path& pathname ) {
    blogSetLinksFetch<orderByTag<post>,filePat>(s,pathname);
}



