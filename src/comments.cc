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

/** Create the comments file if it does not exists and then append
    the comment at the end of the comments file.
 */
class appendPostToFile : public postFilter {
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
	: postFilter(n), commentTop(top), postname(p) {}

    virtual void filters( const post& );
};


void appendPostToFile::filters( const post& p ) {
    using namespace boost::system::errc;
    using namespace boost::filesystem;

    boost::filesystem::ofstream file;
    boost::filesystem::path comments = commentTop / postname;

    if( !boost::filesystem::exists(comments) ) {
	create_directories(comments.parent_path());
	createfile(file,comments);
    } else {
	file.open(comments,std::ios_base::out | std::ios_base::app);
	if( file.fail() ) {
	    boost::throw_exception(basic_filesystem_error<path>(
		std::string("no permissions to open file"),
		postname, 
		make_error_code(no_such_file_or_directory)));
	}
    }

    boost::interprocess::file_lock f_lock(comments.string().c_str());
    f_lock.lock();    
    blogwriter writer(file);
    writer.filters(p);
    file.close();
    f_lock.unlock();

    if( next ) next->filters(p);
}


/** Send the comment post to the SMTP server through the sendmail command.
 */
class sendPostToSMTP : public postFilter {
protected:
    boost::filesystem::path commentTop;
    boost::filesystem::path postname;

public:
   sendPostToSMTP( const boost::filesystem::path& top,
		     const boost::filesystem::path& p ) 
       : commentTop(top), postname(p) {}

    sendPostToSMTP( const boost::filesystem::path& top,
		     const boost::filesystem::path& p, 
		     postFilter* n ) 
	: postFilter(n), commentTop(top), postname(p) {}

    virtual void filters( const post& );
};


void sendPostToSMTP::filters( const post& p ) {
    using namespace boost::system::errc;
    using namespace boost::filesystem;

    std::stringstream cmd;
    cmd << "sendmail";
    FILE *f = popen(cmd.str().c_str(),"rw");
    if( f == NULL ) {
	throw basic_filesystem_error<path>(std::string("executing"),
					   cmd.str(), 
					   make_error_code(no_such_file_or_directory));
    }
    std::stringstream msg;
    msg << "From:" << p.authorEmail << std::endl;
    msg << "Subject:" << p.title << std::endl;
#if 0
    msg << "To:" << "info@" << s.valueOf("domainName")
	<< std::endl << std::endl;
#endif
    msg << p.descr << std::endl;
    fwrite(msg.str().c_str(),msg.str().size(),1,f);
    int err = pclose(f);
    if( err ) {
	throw basic_filesystem_error<path>(std::string("exiting"),
					   cmd.str(), 
					   make_error_code(no_such_file_or_directory));
    }
}


void pageCommentsFetch( session& s, 
			  const boost::filesystem::path& pathname ) 
{
}


void 
commentPage::addSessionVars( boost::program_options::options_description& opts )
{
    using namespace boost::program_options;

    options_description localOptions("comments");
    localOptions.add_options()
	("commentTop",value<std::string>(),"commentTop");
    opts.add(localOptions);
}


void commentPageFetch( session& s, 
			 const boost::filesystem::path& pathname )
{
    url	postname(s.asUrl(boost::filesystem::exists(pathname) ? 
			 pathname : s.abspath(s.valueOf("href"))));

    appendPostToFile comment(s.valueOf("commentTop"),postname.string());

    post p;
    p.authorEmail = s.valueOf("author");
    p.descr = s.valueOf("descr");
    p.time = boost::posix_time::second_clock::local_time();
    comment.filters(p);

    /* \todo clean-up. We use this code such that the browser displays
       the correct url. If we use a redirect, it only works with static 
       pages (index.html). */
    httpHeaders.refresh(0,postname);
}
