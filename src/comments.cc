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
#include "comments.hh"

pathVariable commentTop("commentTop",
			"root of the tree where comments are stored");

void 
commentAddSessionVars( boost::program_options::options_description& opts,
		       boost::program_options::options_description& visible )
{
    using namespace boost::program_options;

    options_description localOptions("comments");
    localOptions.add(commentTop.option());
    opts.add(localOptions);
}


/** Create the comments file if it does not exists and then append
    the comment at the end of the comments file.
 */
class appendPostToFile : public passThruFilter {
protected:
    boost::filesystem::path commentTop;
    boost::filesystem::path postname;

public:
   appendPostToFile( const boost::filesystem::path& top,
		     const boost::filesystem::path& p ) 
       : commentTop(top), postname(p) {}

    appendPostToFile( const boost::filesystem::path& top,
		     const boost::filesystem::path& p, 
		     postFilter* n ) 
	: passThruFilter(n), commentTop(top), postname(p) {}

    virtual void filters( const post& );
};


void appendPostToFile::filters( const post& p ) {
    using namespace boost::system::errc;
    using namespace boost::filesystem;

    boost::filesystem::ofstream file;
    boost::filesystem::path comments = commentTop / postname;

#if 0
    /* \todo How do this? pass session to filter? 
       append and pass stream to filter? */
    s.appendfile(file,comments);
#endif

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
    std::string recipient;

public:
    explicit sendPostToSMTP( const std::string& r ) : recipient(r) {}
    sendPostToSMTP( const std::string& r, postFilter *n ) 
	: passThruFilter(n), recipient(r) {}

    virtual void filters( const post& );
};


void sendPostToSMTP::filters( const post& p ) {
    using namespace boost::system::errc;
    using namespace boost::filesystem;
 
    /* Format the post to be sent as an email. */
    std::stringstream msg;
    mailwriter mail(msg);
    mail.filters(p);
 
    /* \todo For now, execute the sendmail shell command. Later,
             it might be better to communicate with the MTA directly.
    */
    std::stringstream cmd;
    cmd << "sendmail " << recipient;
    FILE *f = popen(cmd.str().c_str(),"rw");
    if( f == NULL ) {
	boost::throw_exception(basic_filesystem_error<path>(
	    std::string("executing"),
	    cmd.str(), 
	    make_error_code(no_such_file_or_directory)));
    }
    fwrite(msg.str().c_str(),msg.str().size(),1,f);
    int err = pclose(f);
    if( err ) {
	boost::throw_exception(basic_filesystem_error<path>(
	    std::string("exiting"),
	    cmd.str(), 
	    make_error_code(no_such_file_or_directory)));
    }
}


void pageCommentsFetch( session& s, 
			  const boost::filesystem::path& pathname ) 
{
}


void commentPage( session& s, 
		  const boost::filesystem::path& pathname )
{
    boost::filesystem::path docname(pathname.parent_path());
    url	postname(s.asUrl(docname));

#if 0
    appendPostToFile comment(commentTop.value(s),postname.string());
#else
    /* \todo Still in testing, we send to info@domainName but the address
       might be worth a toplevel variable... */
    std::stringstream recipient;
    recipient << "info@" << domainName.value(s);
    sendPostToSMTP comment(recipient.str());
#endif

    post p;
    p.authorEmail = authorVar.value(s);
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
