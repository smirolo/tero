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

#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/date_facet.hpp>
#include <Poco/Net/SMTPClientSession.h>
#include <Poco/Net/MailMessage.h>
#include "comments.hh"

pathVariable commentTop("commentTop",
			"root of the tree where comments are stored");
sessionVariable recipient("commentRecipient",
			  "e-mail address comments are sent to");
urlVariable smtpHost("smtpHost",
		       "host to connect to in order to send comments");
intVariable smtpPort("smtpPort",
			 "port to connect to in order to send comments");
sessionVariable smtpLogin("smtpLogin",
			  "login to the smtp server");
sessionVariable smtpPassword("smtpPassword",
			     "password to the smtp server");

void 
commentAddSessionVars( boost::program_options::options_description& opts,
		       boost::program_options::options_description& visible )
{
    using namespace boost::program_options;

    options_description localOptions("comments");
    localOptions.add(commentTop.option());
    localOptions.add(recipient.option());
    localOptions.add(smtpHost.option());
    localOptions.add(smtpPort.option());
    localOptions.add(smtpLogin.option());
    localOptions.add(smtpPassword.option());
    opts.add(localOptions);
}


void appendCommentToFile::filters( const post& p ) {
    using namespace boost::system::errc;
    using namespace boost::filesystem;

    boost::filesystem::ofstream file;
    boost::filesystem::path comments 
	= commentTop.value(*mySession) / (postname.string() + ".comments");

    mySession->appendfile(file,comments);
    boost::interprocess::file_lock f_lock(comments.string().c_str());
    f_lock.lock();    
    mailwriter writer(file);
    writer.filters(p);
    file.close();
    f_lock.unlock();

    if( next ) next->filters(p);
}


/** Send the comment post to the SMTP server through the sendmail command.
 */
class sendPostToSMTP : public passThruFilter {
protected:
    const session* mySession;

public:
    explicit sendPostToSMTP( const session& s ) : mySession(&s) {}
    sendPostToSMTP( const session& s, postFilter *n ) 
	: passThruFilter(n), mySession(&s) {}

    virtual void filters( const post& );
};


void sendPostToSMTP::filters( const post& p ) {
    using namespace boost::system::errc;
    using namespace boost::filesystem;

    Poco::Net::MailMessage message;    

    std::stringstream received;
    received << "from unknown (unknown [" << mySession->client() << "])	by "
	     << domainName.value(*mySession) << " (smailui) with SMTP id "
	     << "XXXXXXXXXXX for <" << recipient.value(*mySession) << ">;"
	     << p.time;
    message.set("Received",received.str());
    message.addRecipient(Poco::Net::MailRecipient(Poco::Net::MailRecipient::PRIMARY_RECIPIENT,recipient.value(*mySession)));
    message.setSubject(p.title);
    message.setSender(p.authorEmail);
    message.setContent(p.content,Poco::Net::MailMessage::ENCODING_QUOTED_PRINTABLE);
#if 0
    message.setContentType(const std::string& mediaType);
    message.setDate(const Poco::Timestamp& dateTime);
#endif
    Poco::Net::SMTPClientSession session(smtpHost.value(*mySession).string(),
					 smtpPort.value(*mySession));
    session.login(Poco::Net::SMTPClientSession::AUTH_LOGIN,
		  smtpLogin.value(*mySession),smtpPassword.value(*mySession));
    session.sendMessage(message);
    session.close();
}


void pageCommentsFetch( session& s, 
			  const boost::filesystem::path& pathname ) 
{
    htmlwriter writer(s.out()); 
    mailParser parser(writer);
    parser.fetch(s,pathname);
}


void commentPage( session& s, 
		  const boost::filesystem::path& pathname )
{
    boost::filesystem::path docname(pathname.parent_path());
    if( !s.valueOf("href").empty() ) {
	docname = s.valueOf("href");
    }

    url	postname(s.asUrl(docname));

    sendPostToSMTP comment(s);

    post p;
    std::stringstream sender;
    sender << authorVar.value(s) 
	   << " <" << s.client() << "@" << domainName.value(s) << ">";
    p.authorEmail = sender.str();
    std::stringstream title;
    title << "Comment on " << postname;
    p.title = title.str();
    p.content = descrVar.value(s);
    p.time = boost::posix_time::second_clock::local_time();
    comment.filters(p);

    /* \todo clean-up. We use this code such that the browser displays
       the correct url. If we use a redirect, it only works with static 
       pages (index.html). */
    httpHeaders.refresh(0,postname);
}
