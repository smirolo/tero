// -*- C++ -*-

#include "markup.hh" 

template<typename cmp>
void blogByOrder<cmp>::provide()
{
    std::sort(indices.begin(),indices.end(),cmp());
    for( indexSet::const_iterator p = indices.begin();
	 p != indices.end(); ++p ) {
	std::cerr << p->time << " - "
		  << p->tag << " - " << p->title << std::endl;
    }
}


template<typename cmp>
void blogByOrder<cmp>::fetch( session& s, 
			      const boost::filesystem::path& pathname )
{
    using namespace boost::gregorian;
    using namespace boost::posix_time;

    blogIndex::fetch(s,pathname);

    provide();

    /* \todo need to specify bounds... 
       [first,last[ range out of session or if does not exist
       get default from cmp operator.
       \todo Need to be able to specify range or start + nbEntries 
       (+max page size?)
    */
    cmp c;

    std::string firstName = boost::filesystem::basename(pathname);

    shortPost bottom = c.first(firstName);
    shortPost top = c.last(firstName);

    /* sorted from decreasing order most recent to oldest. */
    indexSet::const_iterator first 
	= std::lower_bound(indices.begin(),indices.end(),bottom,c);
    if( first == indices.end() ) first = indices.begin();

    indexSet::const_iterator last 
	= std::upper_bound(indices.begin(),indices.end(),top,c);

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

    /** Read the actual blog post file and use an html filter 
	to display it. */
    shortPost prev;
    std::stringstream header;
    time_facet* facet(new time_facet("%d %b %Y"));
    header.imbue(std::locale(header.getloc(), facet));
    for( ; first != last; ++first ) {
	if( first->filename != prev.filename ) { 
	    header.str("");
	    header << html::div()
		   << html::div().classref("title") 
		   << first->title << html::div::end
		   << html::div().classref("date") 
		   << first->time << html::div::end;
	    if( !first->authorName.empty() || !first->authorEmail.empty() ) {
		header << html::div().classref("author") 
		       << first->authorName
		       << " " << first->authorEmail
		       << html::div::end;
	    }
	    header << html::div::end;
	    text t(*ostr,header.str());
	    t.fetch(s,first->filename);
	}
	prev = *first;
    }
}


template<typename cmp>
void blogSetLinks<cmp>::fetch( session& s, 
			       const boost::filesystem::path& pathname )
{
    using namespace boost::gregorian;
    using namespace boost::posix_time;

    blogIndex::fetch(s,pathname);

    cmp c;
    for( indexSet::const_iterator idx = indices.begin(); 
	 idx != indices.end(); ++idx ) {
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
    time_facet* facet(new time_facet("%b %Y"));
    time_facet* linkfacet(new time_facet("%Y-%m-01"));    
    strm.imbue(std::locale(strm.getloc(), linkfacet));
    (*ostr).imbue(std::locale((*ostr).getloc(), facet));
    for( typename linkSet::const_iterator link = links.begin();
	 link != links.end(); ++link ) {
	strm.str("");
	strm << "/" << root << "blog/" << c.name << "/" << link->first;       
	*ostr << html::a().href(strm.str()) 
	      << link->first << " (" << link->second << ")"
	      << html::a::end 
	      << "<br />" << std::endl;
    }
}



