/* Copyright (c) 2009-2010, Fortylines LLC
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

#ifndef guardblog
#define guardblog

#include "post.hh"
#include "document.hh"

/** Pages related to blog posts.

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


/** Load an index of blog entries from a directory and provide them 
    in different sort order. */
class blogIndex : public document {
protected:

    class addPostIndex : public postFilter {
    protected:
	blogIndex* blog;
	
    public:
	explicit addPostIndex( blogIndex& b ) : blog(&b) {}

	virtual void filters( const post& p ) {
	    /* create one shortPost per post tag. */
	    if( p.tags.empty() ) {
		std::cerr << "no tags for " << p.filename << std::endl;
		blog->indices.push_back(shortPost(p,""));
	    } else {
		for( post::tagSet::const_iterator t = p.tags.begin();
		     t != p.tags.end(); ++t ) {
		    blog->indices.push_back(shortPost(p,*t));
		}
	    }
	}
    };

    typedef std::vector<shortPost> indexSet;

    static indexSet indices;

public:
    explicit blogIndex( std::ostream& o ) 
	: document(o) {}

    void fetch( session& s, const boost::filesystem::path& pathname );
};

/** Present the blog entries that match a specific criteria. 
*/
template<typename cmp>
class blogByOrder : public blogIndex {
protected:
    void provide();

public:
    explicit blogByOrder( std::ostream& o ) 
	: blogIndex(o) {}

    void fetch( session& s, const boost::filesystem::path& pathname );
};

typedef blogByOrder<orderByTime<shortPost> > blogByDate;
typedef blogByOrder<orderByTag<shortPost> > blogByTags;
 
/** Links to sets of blog posts sharing a specific key (ie. month, tag, etc.).
*/
template<typename cmp>
class blogSetLinks : public blogIndex {
protected:
    /* store the number blog entries with a specific key. */
    typedef std::map<typename cmp::keyType,uint32_t> linkSet;

    linkSet links;

public:
    explicit blogSetLinks( std::ostream& o ) 
	: blogIndex(o) {}

    void fetch( session& s, const boost::filesystem::path& pathname );
};

typedef blogSetLinks<orderByTime<shortPost> > blogDateLinks;
typedef blogSetLinks<orderByTag<shortPost> > blogTagLinks;

#include "blog.tcc"

#endif
