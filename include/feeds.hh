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

    By default present feed posts in a specific block [base,length[
    where base is an index and length is a number of entries.
 */
class feedIndex : public postFilter {
public:
    typedef std::vector<post> indexSet;
    typedef indexSet::const_iterator const_iterator;
    typedef indexSet::iterator iterator;

    typedef std::iterator_traits<iterator>::difference_type difference_type;

    static indexSet indices;

protected:

    static iterator first;
    static iterator last;

public:
    feedIndex() {}

    explicit feedIndex( postFilter *n ) 
	: postFilter(n) {}

    const_iterator begin() const { return first; }

    const_iterator end() const { return last; }

    iterator begin() { return first; }

    iterator end() { return last; }
    
    virtual void filters( const post& p );
    
    virtual void provide();

    virtual void flush();
};


template<typename cmp>
class feedOrdered : public feedIndex {
public:
    typedef feedIndex super;

public:
    explicit feedOrdered( postFilter *n ) 
	: feedIndex(n) {}

    void provide();
};


template<typename feedBase>
class feedPage : public feedBase {
public:
    typedef feedBase super;

    typedef typename super::difference_type difference_type;

    static const difference_type maxLength;

protected:
    /** index in [first,last[ of the first post displayed
     */
    const difference_type base;

    /** numer of posts displayed from base.
     */
    const difference_type length;

public:
    feedPage() : base(0), length(maxLength) {}

    explicit feedPage( const feedBase& b ) 
	: super(b), base(0), length(maxLength) {}

    feedPage( const feedBase& b, size_t s, int l ) 
	: super(b), base(s), length(l) {}
    
    virtual void provide();
};


template<typename next>
class summarize : next {
public:
    explicit summarize( std::ostream& o ) : next(o) {}

    virtual void filters( const post& v ) {
	post p = v;
	/* \todo picks up first 100 character right now. */
	p.descr = p.descr.substr(0,std::min((size_t)100,p.descr.size()));
	next::filters(p);
    }
};


class feedWriter : public document {
protected:
    static postFilter *feeds;

public:
    virtual void fetch( session& s, const boost::filesystem::path& pathname ) const;
};


/** Aggregate feeds
 */
template<typename postWriter>
class feedAggregate : public feedWriter {
public:
   typedef feedWriter super;

protected:

    /* variable used to match pattern. We cannot use document
     else the catch-all forbids recursive descent through directories. */
    const std::string varname;

    void aggregate( session& s, const boost::filesystem::path& pathname ) const;


public:
    explicit feedAggregate( const std::string& v ) 
	: varname(v) {}

    virtual void fetch( session& s, const boost::filesystem::path& pathname ) const;
};

/** Complete aggregate, does not discard posts. */
typedef feedAggregate<rsswriter> rssAggregate;

/** Feed of text file content from a directory
 */
class feedContent : public feedWriter {
protected:
    typedef feedWriter super;
    boost::regex filePat;

public:
    explicit feedContent(const boost::regex& pat = boost::regex(".*") )
	: filePat(pat) {}

    virtual void fetch( session& s, const boost::filesystem::path& pathname ) const;
};


class htmlContent : public feedContent {
protected:
    typedef feedContent super;

public:
    explicit htmlContent( const boost::regex& pat = boost::regex(".*") ) : feedContent(pat) {}

    virtual void fetch( session& s, const boost::filesystem::path& pathname ) const;
};


class rssContent : public feedContent {
protected:
    typedef feedContent super;

public:
    explicit rssContent( const boost::regex& pat = boost::regex(".*") ) : feedContent(pat) {}

    virtual void fetch( session& s, const boost::filesystem::path& pathname ) const;
};


/** Feed of summaries for files in a directory.
    The summary is based on the first N lines of the file 
    or the filename when there are no assiated presentation.
 */
template<typename postWriter>
class feedSummary : public feedContent {
protected:
    typedef feedContent super;

public:
    explicit feedSummary( const boost::regex& pat = boost::regex(".*") )
	: super(pat) {}

    virtual void fetch( session& s, const boost::filesystem::path& pathname ) const;
};

typedef feedSummary<htmlwriter> htmlSummary;
typedef feedSummary<rsswriter> rssSummary;


/** Feed from a repository commits
 */
template<typename postWriter>
class feedRepository : public feedWriter {
public:
    typedef feedWriter super;

public:
    feedRepository() {}

    virtual void fetch( session& s, const boost::filesystem::path& pathname ) const;	
};

typedef feedRepository<htmlwriter> htmlRepository;
typedef feedRepository<rsswriter> rssRepository;


template<typename postWriter>
class feedLatestPosts : public feedAggregate<postWriter> {
public:
   typedef feedAggregate<postWriter> super;

public:
    explicit feedLatestPosts( const std::string& v );

    virtual void fetch( session& s, const boost::filesystem::path& pathname ) const;	
};

/** Aggregate over time with a fixed number of posts. 
    Most suitable for site feeds. */
typedef feedLatestPosts<htmlwriter> htmlSiteAggregate;
typedef feedLatestPosts<rsswriter> rssSiteAggregate;


#include "feeds.tcc"

#endif
