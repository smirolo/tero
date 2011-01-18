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

    static indexSet indices;

    static const int maxLength;

protected:

    const boost::filesystem::path& pathname;

    /** index in [first,last[ of the first post displayed
     */
    const size_t base;

    /** numer of posts displayed from base.
     */
    const int length;

    static iterator first;
    static iterator last;

public:
    feedIndex( const boost::filesystem::path& p, size_t b, int l ) 
	: pathname(p), base(b), length(l) {}

    const_iterator begin() const { return first; }

    const_iterator end() const { return last; }

    iterator begin() { return first; }

    iterator end() { return last; }
    
    virtual void filters( const post& p );
    
    virtual void provide();

    static feedIndex instance;
};


template<typename cmp>
class feedOrdered : public feedIndex {
public:
    typedef feedIndex super;

public:
    feedOrdered( const boost::filesystem::path& p, size_t b, int l ) 
	: feedIndex(p,b,l) {}

    void provide();
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


template<typename feedReader, typename postWriter>
class feedWriter : public document {
public:
    virtual void fetch( session& s, const boost::filesystem::path& pathname ) const;
};


/** Aggregate feeds
 */
template<typename feedReader, typename postWriter>
class feedAggregate : public feedWriter<feedReader,postWriter> {
public:
    virtual void meta( session& s, const boost::filesystem::path& pathname ) const;
};

/** Complete aggregate, does not discard posts. */
typedef feedAggregate<feedIndex,rsswriter> rssAggregate;

/** Aggregate over time with a fixed number of posts. 
    Most suitable for site feeds. */
typedef feedAggregate<feedOrdered<orderByTime<post> >,
		      htmlwriter> htmlSiteAggregate;
typedef feedAggregate<feedOrdered<orderByTime<post> >,
		      rsswriter> rssSiteAggregate;


/** Feed of text file content from a directory
 */
template<typename feedReader, typename postWriter>
class feedContent : public feedWriter<feedReader,postWriter> {
protected:
    boost::regex filePat;

public:
    explicit feedContent(const boost::regex& pat = boost::regex(".*") )
	: filePat(pat) {}

    virtual void meta( session& s, const boost::filesystem::path& pathname ) const;
};

typedef feedContent<feedIndex,htmlwriter> htmlContent;
typedef feedContent<feedIndex,rsswriter> rssContent;


/** Feed of summaries for files in a directory.
    The summary is based on the first N lines of the file 
    or the filename when there are no assiated presentation.
 */
template<typename feedReader, typename postWriter>
class feedSummary : public feedContent<feedReader,summarize<postWriter> > {
protected:
    typedef feedContent<feedReader,summarize<postWriter> > super;
    boost::regex filePat;

public:
    explicit feedSummary( const boost::regex& pat = boost::regex(".*") )
	: super(pat) {}
};

typedef feedSummary<feedIndex,htmlwriter> htmlSummary;
typedef feedSummary<feedIndex,rsswriter> rssSummary;


/** Feed from a repository commits
 */
template<typename postWriter>
class feedRepository : public feedWriter<feedIndex,postWriter> {
public:
    virtual void meta( session& s, const boost::filesystem::path& pathname ) const;	
};

typedef feedRepository<htmlwriter> htmlRepository;
typedef feedRepository<rsswriter> rssRepository;


#include "feeds.tcc"

#endif
