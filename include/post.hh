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

#ifndef guardpost
#define guardpost

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/gregorian/parsers.hpp>      // from_simple_string
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include "session.hh"

/** A big part of the semilla presentation engine deals with posts. 
    These are snipsets of textual content easily presented through 
    different channels (web, rss feed, e-mail, ...). 
    Blog entries, todo items, mailing lists and repository commits 
    are the major sources of posts.

    Post are usually passed along, transformed and ordered through
    different filters before finishing as formatted html, mail, 
    or rss documents. 
    Functionally there are two behaviors for a filter: pass-thru
    or retained. In pass-thru behavior, the filter triggers *filters()* 
    call to the next filter inside its own *filters()* method. No call
    to *flush()* is necessary. In retained mode, the filter will store
    posts in an internal buffer through its *filters()* method. 
    Processing and calls to the next filter down is done inside 
    the *flush()* method.
    For example, filters that write posts as formatted html are
    typically pass-thru filters while feeds that order posts by
    date work in retained behavior.

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


/** At the heart of every post there is a required date, author and
    full-length textual content.

    In most cases, a post optionaly contains an author's email address,
    a title and a filename the post is derived from. As communication and
    coordination amonst contributors is usually done in the form of posts,
    various scores and tags are also often associated to a post.        
*/
class post {
public:

    // required

    /** The full name of the author of the post. 
     */
    std::string author;

    /** Time at which the post was published.
     */
    boost::posix_time::ptime time;    

    /** The text content of the post. 
     */
    std::string content;

    // optional

    /** The e-mail address of the author of the post.
     */
    std::string authorEmail;    

    /** The title of the post will be displayed as header of pages
       and subject of e-mails. 
    */
    std::string title;   

    /** The post unique identifier is used in many case to associate
	a post to a specific file.

       Implementation Note: 
       At some point the identifier was defined as a boost::uuids::uuid.
       This works great when our application controls the generation
       of identifiers (ex. todo items) but it fails when the format 
       of the identifiers is imposed by an external application (ex. git).
       guid generation. 
    */
    std::string guid;

    /** File this post is derived from. 
     */
    boost::filesystem::path filename;


    // second set of optionals used to implement cool features

    /** Each post as an associated score that can be used to sort
	posts by relevance. Currently, we use the score as the basis
	for the micro-payment system.
     */
    uint32_t score;

    std::string tag;

    /** remove non meaningful whitespaces in the various text fields. 
     */
    void normalize();

    /** returns true if the required fields have been initialized.
     */
    bool valid() const;
};


/** Base class for post filters. 
 */
class postFilter {
protected:

    /** next filter down the chain. 
     */
    postFilter *next;

public:
#if 0
    /* Visible name of the filter

       A mail parser will initialize that name to the pathname 
       of the file being parsed such that todo identifiers can
       be set correctly. */
    std::string name;
#endif

    postFilter() : next(NULL) {}
    explicit postFilter( postFilter *n ) : next(n) {}

    /** Called whenever a new post needs to be processed by the filter.
     */
    virtual void filters( const post& ) = 0;

    /** Called whenever all posts that need processing by the filter
	have been inserted (see *filters()*). 
    */
    virtual void flush() = 0;
};


/** Pass-thru filters process posts as they come through *filters*
    and immediately call the next filter down the chain.
 */
class passThruFilter : public postFilter {
public:
    passThruFilter() {}
    explicit passThruFilter( postFilter *n ) : postFilter(n) {}

    virtual void filters( const post& ) = 0;
    virtual void flush();
};


/** Retained filters aggregate posts in *filters* and process
    them as a single block, calling the next filter down the chain
    in *flush()*.
 */
class retainedFilter : public postFilter {
protected:
    typedef std::vector<post> postSet;
    typedef postSet::iterator iterator;
    typedef postSet::const_iterator const_iterator;

    /** containers of posts in the filter.
     */
    postSet posts;

    /* alias range inside the posts container used in flush to identify
       posts to pass onto the next filter. By default [first,last[ equals
       [posts.begin(),posts.end()[. filter decorators are meant to restrict
       the range to a subset. */
    iterator first;
    iterator last;

    virtual void provide();

public:
    retainedFilter() {}
    explicit retainedFilter( postFilter *n ) : postFilter(n) {}

    virtual void filters( const post& p ) { posts.push_back(p); }

    /** The default implementation of flush is to call provide 
	and push all posts in [first,last[ to the next filter.
    */
    virtual void flush();
};


// ---------------- filters to write posts as formatted html, mail or rss
//                  to a an ostream.


/** Base class to write posts to an ostream.

    Note: This class is purely an implementation artifact (code factorization)
    rather than an interface definition.    
*/
class ostreamWriter : public passThruFilter {
protected:
    /** stream used to write the post to
     */
    std::ostream *ostr;

public:
    explicit ostreamWriter( std::ostream& o ) : ostr(&o) {}
};


/** Write posts as formatted html.
 */
class htmlwriter : public ostreamWriter {
protected:

    /** The counter of posts that have been written so far is used
	to alternate HTML divs between the *postEven* and *postOdd* 
	CSS classes in order to make them more readable.
    */
    size_t postNum;

public:
    explicit htmlwriter( std::ostream& o ) : ostreamWriter(o), postNum(0) {}

    virtual void filters( const post& );
};


/** Write posts as formatted mail (i.e. mbox).
 */
class mailwriter : public ostreamWriter {
public:
    explicit mailwriter( std::ostream& o ) : ostreamWriter(o) {}

    virtual void filters( const post& );
};


/** Write posts as rss items

    Reference: http://www.rssboard.org/rss-specification
    RSS Icons: http://www.feedicons.com/

   Reference documentation says that the RSS documents should be delivered
   with ContentType="application/rss+xml" none-the-less Thunderbird does
   not seem to accept this very well and "text/html; charset=UTF-8"
   might still be more appropriate.
*/
class rsswriter : public ostreamWriter {
public:
    explicit rsswriter( std::ostream& o ) : ostreamWriter(o) {}

    virtual void filters( const post& );
};



/** Comparaison functor based on a post's score.
 */
struct orderByScore : public std::binary_function<post, post, bool> {
    bool operator()( const post& left, const post& right ) const {
	return left.score > right.score;
    }
};


template<typename vT>
struct orderByTime : public std::binary_function<vT,vT,bool> {
    typedef vT valueType;
    typedef boost::posix_time::ptime keyType;

    static const char *name;

    valueType first( const std::string& init = "" ) const {
	using namespace boost::gregorian;
	using namespace boost::posix_time;
	valueType v;
	if( init.empty() ) {
	    v.time = second_clock::local_time();
	} else {
	    v.time = ptime(date(from_simple_string(init)));
	}
	/* decreasing order. */
	v.time = ptime(v.time.date().end_of_month());
	return v;
    }

    valueType last( const std::string& init = "" ) const {
	using namespace boost::gregorian;
	using namespace boost::posix_time;
	valueType v;
	if( init.empty() ) {
	    v.time =  second_clock::local_time();
	} else {
	    v.time = ptime(date(from_simple_string(init)));
	}
	/* decreasing order. */
	v.time = ptime((v.time.date() 
			- boost::gregorian::months(1)).end_of_month());
	return v;
    }
    
    /** The key is made of year and month and used to group posts together.
     */ 
    keyType key( const valueType& p ) const {
	using namespace boost::gregorian;
	try {
	    date d(date_from_tm(boost::posix_time::to_tm(p.time)));	
	    return keyType(date(d.year(),d.month(),1));
	} catch( const std::out_of_range& ) {
	    /* in case it is not-a-date-time */
	}
	return keyType(not_a_date_time);
    }

    bool operator()( const valueType& left, const valueType& right ) const {
	return left.time > right.time;
    }
};

template<typename vT>
const char* orderByTime<vT>::name = "archive";


/** \todo tag value in post indices
*/
template<typename vT>
struct orderByTag : public std::binary_function<vT, vT, bool> {
    typedef vT valueType;
    typedef std::string keyType;

    static const char *name;

    valueType first( const std::string& init = "" ) const {
	valueType v;
	v.tag = init;
	return v;
    }

    valueType last( const std::string& init = "" ) const {
	valueType v;
	v.tag = init;
	return v;
    }

    keyType key( const valueType& p ) const {
	return p.tag;
    }

    bool operator()( const valueType& left, const valueType& right ) const {
	return left.tag > right.tag 
	    || (left.tag > right.tag && left.time > right.time);
    }
};

template<typename vT>
const char* orderByTag<vT>::name = "tags";



extern sessionVariable titleVar;
extern sessionVariable authorVar;
extern sessionVariable authorEmail;
extern sessionVariable descrVar;

void 
postAddSessionVars( boost::program_options::options_description& all,
		    boost::program_options::options_description& visible );


#endif
