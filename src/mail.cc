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

#include <string>
#include <locale>
#include <boost/date_time/local_time/local_time.hpp>
#include <Poco/Net/SMTPClientSession.h>
#include "mail.hh"
#include "markup.hh"
#include "composer.hh"

/** Parser for mailboxes

    references: 
      http://en.wikipedia.org/wiki/Mbox
      see also rfc2822tok.cc

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


urlVariable smtpHost("smtpHost",
		       "host to connect to in order to send emails");
intVariable smtpPort("smtpPort",
			 "port to connect to in order to send emails");
sessionVariable smtpLogin("smtpLogin",
			  "login to the $smtpHost:$smtpPort");
sessionVariable smtpPassword("smtpPassword",
			     "password $smtpHost:$smtpPort");

namespace {

	void validate( const session& s ) {
		assert( !smtpHost.value(s).empty() );
		assert( smtpPort.value(s) != 0);
		assert( !smtpLogin.value(s).empty() );
		assert( !smtpPassword.value(s).empty() );
	}

} // anonymous


void
mailAddSessionVars( boost::program_options::options_description& opts,
					boost::program_options::options_description& visible )
{
    using namespace boost::program_options;

    options_description localOptions("mail");
    localOptions.add(smtpHost.option());
    localOptions.add(smtpPort.option());
    localOptions.add(smtpLogin.option());
    localOptions.add(smtpPassword.option());
    opts.add(localOptions);
}


std::string qualifiedMailAddress( const session& s, const std::string& uid ) {
	std::stringstream qualified;
	qualified << uid << '@' << domainName.value(s);
	return qualified.str();
}


void sendMail( const session& s, const Poco::Net::MailMessage& message )
{
	using namespace Poco::Net;
	validate(s);

	SMTPClientSession session(smtpHost.value(s).string(),
							  smtpPort.value(s));
    session.login(SMTPClientSession::AUTH_LOGIN,
				  smtpLogin.value(s),
				  smtpPassword.value(s));
    session.sendMessage(message);
    session.close();
}


char noTemplate[] = "";

void sendMail( session& s,
			   const std::string& recipient,
			   const std::string& subject,
			   const char *contentTemplate )
{
	using namespace Poco::Net;

	std::ostream& prevDisp = s.out();
	std::stringstream content;
	s.out(content);
	compose<noTemplate>(s,contentTemplate);
	s.out(prevDisp);

    MailMessage message;    
    message.setSubject(subject);
    message.setSender(qualifiedMailAddress(s,"no-reply"));
    message.setContent(content.str(),MailMessage::ENCODING_QUOTED_PRINTABLE);
    message.addRecipient(MailRecipient(MailRecipient::PRIMARY_RECIPIENT,
									   recipient));
	sendMail(s, message);
}	


void mailthread::filters( const post& p ) {
    static const boost::regex titlePat("(^.*:)(.*)");

    boost::smatch m;
    if( boost::regex_match(p.title,m,titlePat) ) {    
	indexMap::iterator found = indexes.find(m.str(2));
	if( found != indexes.end() ) {
	    ++found->second;
	} else {
	    indexes[m.str(2)] = 0;
	}
    }
}


void mailthread::flush() {
    for( indexMap::iterator idx = indexes.begin(); 
	 idx != indexes.end(); ++idx ) {
	*ostr << idx->first << "(" << idx->second << ")" << std::endl;
    }
}


void mailAsPost::newline(const char *line, int first, int last )
{
}


void mailAsPost::token( rfc2822Token token, const char *line, 
			int first, int last, bool fragment )
{
    switch( token ) {
    case rfc2822FieldName:
	field = rfc2822Err;
	if( strncmp(&line[first],"Date",last - first) == 0 ) {
	    field = rfc2822Time;
	} else if( strncmp(&line[first],"From",last - first) == 0 ) {
	    field = rfc2822AuthorEmail;
	} else if( strncmp(&line[first],"Subject",last - first) == 0 ) {
	    field = rfc2822Title;
	} else if( strncmp(&line[first],"Score",last - first) == 0 ) {
	    field = extScore;
	} else {
	    name = std::string(&line[first],last - first);
	    name[0] = std::tolower(name[0]);
	}
	break;
    case rfc2822FieldBody: {
	std::string value = strip(std::string(&line[first],last - first));
	switch( field ) {
	case rfc2822Time:
	    try {
		constructed.time = from_mbox_string(value);
	    } catch( std::exception& e ) {
		std::cerr << "unable to reconstruct time from " << value << std::endl;
	    }
	    break;
	case rfc2822AuthorEmail:
	    constructed.author = extractName(value);
	    constructed.authorEmail = extractEmailAddress(value);	    
	    break;
	case rfc2822Title:
	    constructed.title = value;
	    break;
    case extScore:
        constructed.score = atoi(value.c_str());
        break;
	default: {
	    post::headersMap::iterator header 
		= constructed.moreHeaders.find(name);
	    if( header != constructed.moreHeaders.end() ) {
		header->second += std::string(",") + value;
	    } else {
		constructed.moreHeaders[name] = value;
	    }
	} break;
	}
    } break;
    case rfc2822MessageBody:
	constructed.content = std::string(&line[first],&line[last]);
	break;
    default:
	/* to stop gcc from complaining */
	break;
    }    
}


void mailParser::walk( session& s, std::istream& ins, const std::string& name ) const
{
    using namespace boost::gregorian;
    using namespace boost::posix_time;
    using namespace boost::local_time;
    using namespace std;

    typedef std::set<std::string> tagSet;

    /** tags associated to a post. 
     */
    size_t lineCount = 0;
    mailAsPost listener;
    rfc2822Tokenizer tok(listener);
    std::stringstream mailtext;
    while( !ins.eof() ) {
		std::string line;
		std::getline(ins,line);
		++lineCount;
		
		if( line.compare(0,5,"From ") == 0 ) {
			/* Beginning of new message
			   http://en.wikipedia.org/wiki/Mbox */
			if( !mailtext.str().empty() ) {
				/* First line of file introduces first e-mail. */
				if( stopOnFirst ) break;	    
				tok.tokenize(mailtext.str().c_str(),mailtext.str().size());
				post p = listener.unserialized();
				p.guid = s.asUrl(name).string();
				filter->filters(p);
				mailtext.str("");
			}
		} else {
			mailtext << line << std::endl;
		}
    }
    tok.tokenize(mailtext.str().c_str(),mailtext.str().size());
    post p = listener.unserialized();
    p.guid = s.asUrl(name).string();
    filter->filters(p);
    mailtext.str("");    
}


void mailParserFetch( session& s, const boost::filesystem::path& pathname )
{
    mailthread mt(s.out());
    mailParser mp(mt);
    mp.fetch(s,pathname);
#if 0
    /* \todo Cannot flush here if we want to support feedContent calls blogEntry.
       This call will generate non-intended intermediate flushes. It might be
       useful for other parts of the system though...
     */
    filter->flush();
#endif
}
