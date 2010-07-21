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

#ifndef guardmail
#define guardmail

#include "document.hh"
#include "post.hh"

/* A *mailthread* filter attempts to gather all mails that appear 
   to belong to the same thread together. */
class mailthread : public postFilter {
protected:
    typedef std::map<std::string,uint32_t> indexMap;

    indexMap indexes;

public:
    mailthread() {}

    explicit mailthread( postFilter *n ) : postFilter(n) {}

    virtual void filters( const post& );
    virtual void flush();
};


class mailParser : public dirwalker {
protected:
    enum parseState {
	startParse,
	dateParse,
	authorParse,
	titleParse
    };

    postFilter *filter;

public:
    explicit mailParser( postFilter& f ) : filter(&f) {}

    virtual void fetch( session& s, const boost::filesystem::path& pathname );

    void walk( session& s, std::istream& ins, const std::string& name = "" );
};


#endif
