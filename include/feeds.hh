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
#include "decorator.hh"

/* Feeds are used to bundle and present a set of posts together.
   The aggregate function is used to populate a feed from different
   sources selected through the toplevel *dispatchDoc*.
   The ordered and page retained post filters are usually used 
   together to display an ordered subset of posts (example: the latest
   commits).
   The summurize and oneline pass-thru filters implement alternative
   outputs to the default (html, rss, etc.) writers. The summurize
   filter is particularly useful to produce rss feeds that only contain
   the first paragraph of a post. The oneline filter is used to generate
   rows of a table, for example a list of todo items.

   Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


/** On flush (i.e. provide), the feedOrdered retained filter will sort 
    its posts using the *cmp* operator and pass it to the next filter 
    down in sorted order. */
template<typename cmp>
class feedOrdered : public retainedFilter {
public:
    typedef retainedFilter super;
    typedef std::iterator_traits<iterator>::difference_type difference_type;

    virtual void provide();

public:
    feedOrdered() {}
    explicit feedOrdered( postFilter *n ) : super(n) {}    
};


/** Select a subset of posts. */
template<typename cmp>
class feedSelect : public passThruFilter {
public:
    typedef passThruFilter super;
    typedef std::set<typename cmp::keyType> matchKeySet;
    matchKeySet matchKeys;

public:
    feedSelect() {}
    explicit feedSelect( const matchKeySet& keys ) : matchKeys(keys) {}
    feedSelect( postFilter *n, const matchKeySet& keys )
        : super(n), matchKeys(keys) {}

    virtual void filters( const post& );  
};


/** Compact consecutive posts that share the same guid.
 */
class feedCompact : public passThruFilter {
public:
    typedef passThruFilter super;
	std::string prev;
	bool prevInit;

public:
    feedCompact() : prevInit(false) {}
    explicit feedCompact( postFilter *n ) : super(n), prevInit(false) {}

    virtual void filters( const post& );  
};


/** feedPage is a filter decorator that constraint the flush of its 
    parent class filter (feedBase) to a single page of posts.
*/
template<typename feedBase>
class feedPage : public feedBase {
public:
    typedef feedBase super;

    typedef typename super::difference_type difference_type;

    static const difference_type maxLength;

protected:
    /** index of the first post displayed (i.e. pageNum * pageLength)
     */
    const difference_type base;

    /** numer of posts displayed per page
     */
    const difference_type length;

    virtual void provide();

public:
    feedPage() : base(0), length(maxLength) {}

    explicit feedPage( const feedBase& b ) 
	: super(b), base(0), length(maxLength) {}

    feedPage( const feedBase& b, size_t pageLength ) 
	: super(b), base(0), length(pageLength) {}

    feedPage( const feedBase& b, size_t pageLength, size_t pageNum ) 
	: super(b), base(pageNum * pageLength), length(pageLength) {}
};


/** Filter to shorten the content of a post to the first N characters.
 */
class summarize : public passThruFilter {
protected:
    size_t length;

public:
    summarize() : length(100) {}
    explicit summarize( postFilter *n ) : passThruFilter(n), length(100) {}
    summarize( postFilter *n, size_t l ) : passThruFilter(n), length(l) {}

    virtual void filters( const post& p );
};


/** Write an html table row with author, title and score.
 */
class oneliner : public ostreamWriter {
public:
    explicit oneliner( std::ostream& o ) : ostreamWriter(o) {}

    virtual void filters( const post& );
};


/** Write an html table row per post; group all posts with the same date
	under a single header.
 */
class byTimeHtml : public ostreamWriter {
private:
    /** Time stored in the previous post (used in filters method).
     */
    boost::posix_time::ptime prev_header;    


public:
    explicit byTimeHtml( std::ostream& o ) : ostreamWriter(o) {}

    virtual void filters( const post& );
};



/** Aggregate feeds

    Use the toplevel *dispatchDoc* to fetch callback *varname*, for a file 
    basename(pathname) in all sub-directories in dirname(pathname).

    This function uses dispatchDoc::fetch and thus potentially might be 
    invoked recursively. All posts are passed through the global s.feeds 
    so it remains to insure s.feeds is not intempestively updated.
    We thus only initialized s.feeds to defaultWriter and later call 
    s.feeds.flush when it is null entering the function.
*/
template<typename defaultWriter, const char* varname, const char *filePat>
void feedAggregate( session& s, const boost::filesystem::path& pathname );


/** regular expression pattern that matches all files.
 */
extern char allFilesPat[];

/** Generate a feed of all files matching *filePat* in pathname.
 */
template<typename defaultWriter, const char* filePat>
void feedContent( session& s, const boost::filesystem::path& pathname );


/** Feed of summaries for files in a directory.
    The summary is based on the first N lines of the file 
    or the filename when there are no assiated presentation.
 */
template<typename defaultWriter, const char* filePat>
void feedSummary( session& s, const boost::filesystem::path& pathname );



/* class feedLatestPosts : public feedAggregate<postWriter> */
template<typename defaultWriter, const char* varname>
void feedLatestPosts( session& s, const boost::filesystem::path& pathname );


/** Aggregate over time with a fixed number of posts. 
    Most suitable for site feeds. */
template<const char* varname>
void htmlSiteAggregate( session& s, const boost::filesystem::path& pathname );

template<const char* varname>
void rssSiteAggregate( session& s, const boost::filesystem::path& pathname );


#include "feeds.tcc"

#endif
