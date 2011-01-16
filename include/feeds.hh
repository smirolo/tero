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

#ifndef guardfeeds
#define guardfeeds

#include "mail.hh"
#include "post.hh"

/* Feeds can be generated out of repository commits or directories,
   aggregated together and presented as HTML or RSS.

   Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


/** Index of post entries
 */
class feedIndex : public postFilter {
public:
    typedef std::vector<shortPost> indexSet;
    
    indexSet indices;
    
public:
    
    virtual void filters( const post& p ) {
	/* create one shortPost per post tag. */
	if( p.tags.empty() ) {
	    indices.push_back(shortPost(p,""));
	} else {
	    for( post::tagSet::const_iterator t = p.tags.begin();
		 t != p.tags.end(); ++t ) {
		indices.push_back(shortPost(p,*t));
	    }
	}
    }

    static feedIndex instance;

};


class feedBase : public document {
public:
    void write( feedIndex::indexSet::const_iterator first,
		feedIndex::indexSet::const_iterator last,
		postFilter& writer ) const;
		
};


/** Aggregate feeds
 */
template<typename postFilter>
class feedAggregate : public feedBase {
public:
    virtual void meta( session& s, const boost::filesystem::path& pathname ) const;

    virtual void fetch( session& s, const boost::filesystem::path& pathname ) const;
};


typedef feedAggregate<htmlwriter> htmlAggregate;
typedef feedAggregate<rsswriter> rssAggregate;


/** Feed of text file content from a directory
 */
template<typename postFilter>
class feedContent : public feedBase {
protected:
    boost::regex filePat;

public:
    explicit feedContent(const boost::regex& pat = boost::regex(".*") )
	: filePat(pat) {}

    virtual void meta( session& s, const boost::filesystem::path& pathname ) const;

    virtual void fetch( session& s, const boost::filesystem::path& pathname ) const;
};

typedef feedContent<htmlwriter> htmlContent;
typedef feedContent<rsswriter> rssContent;


/** Feed of filenames from a directory 
 */
template<typename postFilter>
class feedNames : public dirwalker {
protected:
    boost::regex filePat;

public:
    explicit feedNames( const boost::regex& pat = boost::regex(".*") )
	: filePat(pat) {}

    virtual void meta( session& s, const boost::filesystem::path& pathname ) const;

    virtual void fetch( session& s, const boost::filesystem::path& pathname ) const;
};

typedef feedNames<htmlwriter> htmlNames;
typedef feedNames<rsswriter> rssNames;


/** Feed from a repository commits
 */
template<typename postFilter>
class feedRepository : public feedBase {
public:
    virtual void meta( session& s, const boost::filesystem::path& pathname ) const;	

    virtual void fetch( session& s, const boost::filesystem::path& pathname ) const;
};

typedef feedRepository<htmlwriter> htmlRepository;
typedef feedRepository<rsswriter> rssRepository;


#include "feeds.tcc"

#endif
