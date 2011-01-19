// -*- C++ -*-
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
    cmp c;
    std::string firstName = boost::filesystem::basename(pathname);
    std::sort(indices.begin(),indices.end(),c);
    typename cmp::valueType bottom = c.first(firstName);
    typename cmp::valueType top = c.last(firstName);

    /* sorted from decreasing order most recent to oldest. */
    first = std::lower_bound(indices.begin(),indices.end(),bottom,c);
    if( first == indices.end() ) {
	first = indices.begin();
    }
    last = std::upper_bound(indices.begin(),indices.end(),top,c);

    super::provide();
}


template<typename cmp>
void blogSetLinks<cmp>::fetch( session& s, 
			       const boost::filesystem::path& pathname ) const
{
    using namespace boost::gregorian;
    using namespace boost::posix_time;

    super::fetch(s,pathname);

    cmp c;
    boost::filesystem::path 
	root = s.subdirpart(s.valueOf("siteTop"),
			    s.root(pathname,"blog"));

    /* We donot use pubDate::format here because the code logic relies
       on special formatted links to create subsequent pages. */
    time_facet* facet(new time_facet("%b %Y"));
    time_facet* linkfacet(new time_facet("%Y-%m-01"));    

    std::stringstream strm;
    strm.imbue(std::locale(strm.getloc(), linkfacet));
    s.out().imbue(std::locale(s.out().getloc(), facet));

    /* store the number blog entries with a specific key. */
    typedef std::map<typename cmp::keyType,uint32_t> linkSet;

    linkSet links;
    for( feedIndex::indexSet::const_iterator idx = feedIndex::instance.indices.begin(); 
	 idx != feedIndex::instance.indices.end(); ++idx ) {
	typename linkSet::iterator l = links.find(c.key(*idx));
	if( l != links.end() ) {
	    ++l->second;
	} else {
	    links[c.key(*idx)] = 1;
	}
    }

    /* Display keys and the associated number of blog entries */
    for( typename linkSet::const_iterator link = links.begin();
	 link != links.end(); ++link ) {
	strm.str("");
	strm << "/" << root << "blog/" << c.name << "/" << link->first;       
	s.out() << html::a().href(strm.str()) 
		<< link->first << " (" << link->second << ")"
		<< html::a::end << html::linebreak;
    }
}



