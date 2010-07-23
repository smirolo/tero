/* Copyright (c) 2009, Sebastien Mirolo
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

   THIS SOFTWARE IS PROVIDED BY Sebastien Mirolo ''AS IS'' AND ANY
   EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL Sebastien Mirolo BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#ifndef guardpost
#define guardpost

#include <boost/date_time.hpp>
#include <boost/uuid/uuid.hpp>

boost::uuids::uuid asuuid( const std::string& s );

class post {
public:
    uint32_t score;
    boost::uuids::uuid tag;
    std::string authorName;
    std::string authorEmail;    
    std::string title;
    boost::posix_time::ptime time;    
    std::string descr;

    /** remove non meaningful whitespaces from the *author* and *title* fields. 
     */
    void normalize();

    bool valid() const;
};


struct orderByScore : public std::binary_function<post, post, bool> {
    bool operator()( const post& left, const post& right ) const {
	return left.score > right.score;
    }
};


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


#endif
