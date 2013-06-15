/* Copyright (c) 2009-2013, Fortylines LLC
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
    for( typename super::const_iterator f = super::posts.begin();
		 f != super::posts.end(); ++f ) {
		std::cerr << f->time << ": " << f->tag << ": " << f->title;
		if( c(*super::first,*f)
			&& (super::last == super::posts.end() || c(*f,*super::last)) ) {
			std::cerr << " *";
		}
		std::cerr << std::endl;
    }
#endif
}


template<typename defaultWriter, const char* varname, const char* filePat,
         typename cmp>
void blogByInterval( session& s, const url& name )
{
    cmp c;
    defaultWriter writer(s.out());
    typename cmp::valueType lower, upper;
    try {
        boost::smatch m;
#if 0
        std::string firstName = boost::filesystem::basename(name);
#else
        std::string firstName;
        if( boost::regex_search(name.string(),m,
                boost::regex(std::string(c.name) + "-(.*)")) ) {
            firstName = m.str(1);
        }
#endif
        lower = c.first(firstName);
        upper = c.last(firstName);
    } catch( std::exception& e ) {
        lower = c.first();
        upper = c.last();
    }
#if 0
    std::cerr << "[blogByInterval] from " << lower.time << " to " << upper.time
              << "(from " << lower.tag << " to " << upper.tag << ")"
              << std::endl;
#endif
    blogInterval<cmp> interval(&writer,lower,upper);
    blogSplat<cmp> feeds(&interval);
    if( !s.feeds ) {
        s.feeds = &feeds;
    }

    feedContent<defaultWriter,filePat>(s,s.abspath(name));

    if( s.feeds == &feeds ) {
        s.feeds->flush();
        s.feeds = NULL;
    }
}

template<const char* varname, const char* filePat>
void blogByIntervalDate( session& s, const url& name )
{
    blogByInterval<htmlwriter,varname,filePat,orderByTime<post> >(s,name);
}

template<const char* varname, const char* filePat>
void blogByIntervalTags( session& s, const url& name )
{
    blogByInterval<htmlwriter,varname,filePat,orderByTag<post> >(s,name);
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

    std::stringstream strm;
    strm.imbue(std::locale(strm.getloc(), linkfacet));
    ostr->imbue(std::locale(ostr->getloc(), facet));

    /* Display keys and the associated number of blog entries */
    for( typename linkSet::const_iterator link = links.begin();
	 link != links.end(); ++link ) {
	strm.str("");
	strm << "-" << link->first;
	*ostr << "<li>" << html::a().href(root.string() + strm.str())
		<< link->first << " (" << link->second << ")"
		<< html::a::end << "</li>";
    }
}


template<typename cmp, const char *filePat>
void blogSetLinksFetch( session& s, const url& name )
{
    boost::filesystem::path blogroot
        = s.root(s.abspath(name),blogTrigger,true);

    bySet<cmp> count(s.out(),s.asUrl(blogroot));
    blogSplat<cmp> feeds(&count);

    if( !s.feeds ) {
        s.feeds = &feeds;
    }

    /* workaround Ubuntu/gcc-4.4.3 is not happy with boost::regex(filePat)
       passed as parameter to parser. */
    boost::regex pat(filePat);
    mailParser parser(pat,*s.feeds,false);
    parser.fetch(s, blogroot);

    if( s.feeds == &feeds ) {
#if 0
        // XXX There is already one flush() in parser.fetch().
        // Another one here would show the tags repeated twice.
        s.feeds->flush();
#endif
        s.feeds = NULL;
    }
}


template<const char *filePat>
void blogDateLinks( session& s, const url& name ) {
    blogSetLinksFetch<orderByTime<post>,filePat>(s,name);
}

template<const char *filePat>
void blogTagLinks( session& s, const url& name ) {
    blogSetLinksFetch<orderByTag<post>,filePat>(s,name);
}


template<const char *filePat>
void blogRelatedSubjects( session& s, const url& name )
{
    using namespace boost::filesystem;

    boost::filesystem::path blogroot
        = s.root(s.abspath(name), blogTrigger, true);

    post p;
    if( !is_regular_file(s.abspath(name)) ) {
        /* If *name* is not a regular file, we will build a list
           of posts related to the most recent post. */
        mostRecentFilter mostRecent(NULL);
        mailParser walker(boost::regex(blogPat), mostRecent, true);
        walker.fetch(s, blogroot);
        p = mostRecent.mostRecent();
    } else {
        boost::filesystem::path related = name.pathname;
        slice<char> text = s.loadtext(related);
        mailAsPost listener;
        rfc2822Tokenizer tok(listener);
        tok.tokenize(text.begin(),text.size());
        p = listener.unserialized();
    }

    typename feedSelect<orderByTag<post> >::matchKeySet tags;
    insertItems(p.moreHeaders["tags"].begin().base(),
        p.moreHeaders["tags"].end().base(),
        std::inserter(tags,tags.begin()));

#if 0
	std::cerr << "tags=" << std::endl;
	for( typename feedSelect<orderByTag<post> >::matchKeySet::const_iterator 
			 mki = tags.begin(); mki != tags.end(); ++mki ) {
		std::cerr << "\t" << *mki << std::endl;
	}
#endif

    subjectWriter writer(s.out());
	feedCompact distincts(&writer);
    feedSelect<orderByTag<post> > selects(&distincts,tags);
	blogSplat<orderByTag<post> > feeds(&selects);

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


