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

    typedef std::vector<shortPost> indexSet;

    class addPostIndex : public postFilter {
    protected:
	indexSet* indices;
	
    public:
	explicit addPostIndex( indexSet& v ) : indices(&v) {}

	virtual void filters( const post& p ) {
	    /* create one shortPost per post tag. */
	    if( p.tags.empty() ) {
		indices->push_back(shortPost(p,""));
	    } else {
		for( post::tagSet::const_iterator t = p.tags.begin();
		     t != p.tags.end(); ++t ) {
		    indices->push_back(shortPost(p,*t));
		}
	    }
	}
    };

    static indexSet indices;

public:
    void fetch( session& s, const boost::filesystem::path& pathname ) const;
};

/** Present blog entries ordered by *cmp*.
*/
template<typename cmp>
class blogByOrder : public blogIndex {
protected:
    void provide() const;

    void write( session& s,
		indexSet::const_iterator first,
		indexSet::const_iterator last ) const;
};


/** Present blog entries in a specific interval [first,last[
    where first and last are based on *cmp*.
*/
template<typename cmp>
class blogByInterval : public blogByOrder<cmp> {
protected:
    typedef blogByOrder<cmp> super;

public:
    void fetch( session& s, const boost::filesystem::path& pathname ) const;
};


typedef blogByInterval<orderByTime<shortPost> > blogByIntervalDate;
typedef blogByInterval<orderByTag<shortPost> > blogByIntervalTags;


/** Present blog entries in a specific block [base,length[
    where base is an index and length is a number of entries.
*/
template<typename cmp>
class blogByBlock : public blogByOrder<cmp> {
protected:
    typedef blogByOrder<cmp> super;

public:
    void fetch( session& s, const boost::filesystem::path& pathname ) const;
};

typedef blogByBlock<orderByTime<shortPost> > blogByBlockDate;
typedef blogByBlock<orderByTag<shortPost> > blogByBlockTags;

 
/** Links to sets of blog posts sharing a specific key (ie. month, tag, etc.).
*/
template<typename cmp>
class blogSetLinks : public blogIndex {
protected:
    /* store the number blog entries with a specific key. */
    typedef std::map<typename cmp::keyType,uint32_t> linkSet;

    mutable linkSet links;

public:
    void fetch( session& s, const boost::filesystem::path& pathname ) const;
};

typedef blogSetLinks<orderByTime<shortPost> > blogDateLinks;
typedef blogSetLinks<orderByTag<shortPost> > blogTagLinks;

#include "blog.tcc"

#endif
