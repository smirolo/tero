// -*- C++ -*-

#include "markup.hh" 


template<typename cmp>
void blogByInterval<cmp>::fetch( session& s, 
			       const boost::filesystem::path& pathname ) const
{
    cmp c;
    feedIndex::instance.provide(c);

    /* \todo need to specify bounds... 
       [first,last[ range out of session or if does not exist
       get default from cmp operator.
       \todo Need to be able to specify range or start + nbEntries 
       (+max page size?)
    */
    std::string firstName = boost::filesystem::basename(pathname);

    typename cmp::valueType bottom = c.first(firstName);
    typename cmp::valueType top = c.last(firstName);

    /* sorted from decreasing order most recent to oldest. */
    feedIndex::indexSet::const_iterator first 
	= std::lower_bound(feedIndex::instance.indices.begin(),
			   feedIndex::instance.indices.end(),
			   bottom,c);
    if( first == feedIndex::instance.indices.end() ) {
	first = feedIndex::instance.indices.begin();
    }

    feedIndex::indexSet::const_iterator last 
	= std::upper_bound(feedIndex::instance.indices.begin(),
			   feedIndex::instance.indices.end(),top,c);
#if 0
   std::cerr << "!!! bottom: " << bottom.time
	      << ", first: " << first->time
	      << ", top: " << top.time
	     << ", last: " << (( last == indices.end() ) ? boost::posix_time::ptime() : last->time)
	      << std::endl;

    std::cerr << "!!! 2. bottom: " << bottom.tag
	      << ", first: " << first->tag
	      << ", top: " << top.tag
	      << ", last: " << (( last == indices.end() ) ? "end" : last->tag)
	      << std::endl;
#endif
    htmlwriter writer(s.out());
    super::write(first,last,writer);
}


template<typename cmp>
void blogSetLinks<cmp>::fetch( session& s, 
			       const boost::filesystem::path& pathname ) const
{
    using namespace boost::gregorian;
    using namespace boost::posix_time;

    cmp c;
    for( feedIndex::indexSet::const_iterator idx = feedIndex::instance.indices.begin(); 
	 idx != feedIndex::instance.indices.end(); ++idx ) {
	typename linkSet::iterator l = links.find(c.key(*idx));
	if( l != links.end() ) {
	    ++l->second;
	} else {
	    links[c.key(*idx)] = 1;
	}
    }

    boost::filesystem::path 
	root = s.subdirpart(s.valueOf("srcTop"),
			    s.root(pathname,"blog"));

    /* Display keys and the associated number of blog entries */
    std::stringstream strm;
    /* We donot use pubDate::format here because the code logic relies
       on special formatted links to create subsequent pages. */
    time_facet* facet(new time_facet("%b %Y"));
    time_facet* linkfacet(new time_facet("%Y-%m-01"));    
    strm.imbue(std::locale(strm.getloc(), linkfacet));
    s.out().imbue(std::locale(s.out().getloc(), facet));
    for( typename linkSet::const_iterator link = links.begin();
	 link != links.end(); ++link ) {
	strm.str("");
	strm << "/" << root << "blog/" << c.name << "/" << link->first;       
	s.out() << html::a().href(strm.str()) 
		<< link->first << " (" << link->second << ")"
		<< html::a::end 
		<< "<br />" << std::endl;
    }
}



