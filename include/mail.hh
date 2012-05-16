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
	rfc2822Title
    } field;
    
    std::string name;
    post constructed;
    
public:
    mailAsPost() { constructed.score = 0; }

    void newline(const char *line, int first, int last );

    void token( rfc2822Token token, const char *line, 
		int first, int last, bool fragment );

    const post& unserialized() const { return constructed; }
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

    /** Stop parsing after the first post is completed. 
	(This is for todo items with embed comments).
     */
    bool stopOnFirst;

public:
    mailParser( postFilter& f, bool sof = false ) 
	: filter(&f), stopOnFirst(sof) {}
    
    mailParser( const boost::regex& fm, 
		postFilter& f, bool sof = false )
	: dirwalker(fm), filter(&f), stopOnFirst(sof) {}

    virtual void walk( session& s, std::istream& ins, const std::string& name ) const;

};

void mailParserFetch( session&s, const boost::filesystem::path& pathname );

#endif
