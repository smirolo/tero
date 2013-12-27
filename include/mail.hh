/* Copyright (c) 2009-2013, Fortylines LLC
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

#ifndef guardmail
#define guardmail

#include "document.hh"
#include "rfc2822tok.hh"
#include "post.hh"
#include <Poco/Net/MailMessage.h>

/**
   Parses e-mail messages

   Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/

namespace tero {

void
mailAddSessionVars( boost::program_options::options_description& opts,
    boost::program_options::options_description& visible );


/* return a fully qualified mail dddress for delivery to the local SMTP server.
 */
std::string qualifiedMailAddress( const session& s, const std::string& uid );

void sendMail( const session& s, const Poco::Net::MailMessage& message );

void sendMail( session& s,
    const std::string& recipient,
    const std::string& subject,
    const char *contentTemplate );

/* A *mailthread* filter attempts to gather all mails that appear
   to belong to the same thread together. */
class mailthread : public postFilter {
protected:
    typedef std::map<std::string,uint32_t> indexMap;

    indexMap indexes;
    std::ostream* ostr;

public:
    explicit mailthread( std::ostream& o ) : ostr(&o) {}

    mailthread( std::ostream& o, postFilter *n )
        : postFilter(n), ostr(&o) {}

    virtual void filters( const post& );
    virtual void flush();
};


/** Unserialize a post instance out of a rfc2822 e-mail stored as text.
 */
class mailAsPost : public rfc2822TokListener {
protected:
    enum {
        rfc2822Err,
        rfc2822Time,
        rfc2822Content,
        rfc2822AuthorEmail,
        rfc2822Title,
        extScore
    } field;

    /* field name */
    std::string name;

    /* post being reconstructed */
    post constructed;

public:
    /* indicates we actually did something and not just got a messageBreak
       (mbox files start with a messageBreak). */
    bool nontrivial;

public:
    mailAsPost() : nontrivial(false) { constructed.score = 0; }

    void newline(const char *line, int first, int last );

    void token( rfc2822Token token, const char *line,
        int first, int last, bool fragment );

    const post& unserialized() const { return constructed; }
};


/** Walks a directory and invokes a filter on all posts
    in that directory. */
class mailParser : public dirwalker {
protected:
    postFilter *filter;

    /** Stop parsing after the first post is completed.
        (This is for todo items with embed comments).
    */
    bool stopOnFirst;

    virtual void
    addFile( session& s, const boost::filesystem::path& pathname ) const;

    virtual void flush( session& s ) const;

public:
    mailParser( postFilter& f, bool sof = false )
        : filter(&f), stopOnFirst(sof) {}

    mailParser( const boost::regex& fm, postFilter& f, bool sof = false )
        : dirwalker(fm), filter(&f), stopOnFirst(sof) {}

    void fetchFile( session& s, const boost::filesystem::path& pathname ) const
    {
        addFile(s, pathname);
    }

};

void mailParserFetch( session&s, const url& name );

}

#endif
