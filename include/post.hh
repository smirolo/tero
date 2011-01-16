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

#ifndef guardpost
#define guardpost

#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>


/** Snipset of information easily presented through different 
    channels (web, rss feed, e-mail, ...).
    
    Blog entries, todo items and repository commits are all posts.
    At the base, a post is made of a title, an author and some text
    content. It also contains a unique identifier to facilitate
    channel processing.

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


/** Base class with common information present in every post
    such as date, title and author. */
class postBase {
public:
    /** File this short post is derived from. 
     */
    boost::filesystem::path filename;

    /** The full name of the author of the post. 
     */
    std::string authorName;

    /** The e-mail address of the author of the post.
     */
    std::string authorEmail;    

    /** Time at which the post was published.
     */
    boost::posix_time::ptime time;    

    /** The title of the post will be displayed as header of pages
       and subject of e-mails. 
    */
    std::string title;   
};


class post;   // forward declaration


/** Short posts are used to construct sets of links to actual post content.
    Outside the common information (author, time), tags are usually used 
    to filter posts and thus every tag associated to a post ends up generating
    as many shortPosts.
 */
class shortPost : public postBase {
public:
    std::string tag;

    shortPost() {}

    shortPost( const post& p, const std::string& t );

};


/** Full-length post with textual content. 
*/
class post : public postBase {
public:
    typedef std::set<std::string> tagSet;

public:
    std::string tag;

    post() {}

    post( const post& p, const std::string& t );

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

    /** The text content of the post. 
     */
    std::string descr;

    /** Each post as an associated score that can be used to sort
	posts by relevance. Currently, we use the score as the basis
	for the micro-payment system.
     */
    uint32_t score;

    /** tags associated to a post. 
     */
    tagSet tags;

    static void 
    addSessionVars( boost::program_options::options_description& opts );

    /** remove non meaningful whitespaces from the *author* and *title* fields. 
     */
    void normalize();

    bool valid() const;
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


class postFilter {
protected:
    postFilter *next;

public:
    /* Visible name of the filter

       A mail parser will initialize that name to the pathname 
       of the file being parsed such that todo identifiers can
       be set correctly. */
    std::string name;

    postFilter() : next(NULL) {}

    explicit postFilter( postFilter *n ) : next(n) {}

    virtual void filters( const post& );
    virtual void flush();
};

/* buffers a set posts. */
class postBuffer : public postFilter {
protected:
    typedef std::vector<post> postSet;

public:
    postSet posts;

    postBuffer() : postFilter(NULL) {}

    virtual void filters( const post& );
    virtual void flush();

};

class htmlwriter : public postFilter {
protected:
    /** stream used to write the post to
     */
    std::ostream *ostr;

    /** The counter of posts that have been written so far is used
	to alternate HTML divs between the *postEven* and *postOdd* 
	CSS classes in order to make them more readable.
    */
    size_t postNum;

public:
    explicit htmlwriter( std::ostream& o ) : ostr(&o), postNum(0) {}

    virtual void filters( const post& );
};


class blogwriter : public postFilter {
protected:
    std::ostream *ostr;

public:
    explicit blogwriter( std::ostream& o ) 
	: ostr(&o) {}

    virtual void filters( const post& );
};


/** Mark-up a post with RSS element tags 
    
    Reference: http://www.rssboard.org/rss-specification
    RSS Icons: http://www.feedicons.com/

   Reference documentation says that the RSS documents should be delivered
   with ContentType="application/rss+xml" none-the-less Thunderbird does
   not seem to accept this very well and "text/html; charset=UTF-8"
   might still be more appropriate.
*/
class rsswriter : public postFilter {
protected:
    /** stream used to write the post to
     */
    std::ostream *ostr;

public:
    explicit rsswriter( std::ostream& o ) : ostr(&o) {}

    virtual void filters( const post& );
};


#endif
