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

#include "blog.hh"
#include "mail.hh"
#include <boost/regex.hpp>

/** Pages related to blog posts.

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/

char blogPat[] = ".*\\.blog$";
const char *blogTrigger = "blog";


const boost::filesystem::path
mostRecentBlogEntry( session& s, const boost::filesystem::path& pathname )
{
    /* \todo This should take into account the *date* inside the post,
       not just the timestamp on the file. */
    using namespace boost::filesystem;

    boost::filesystem::path blogroot
        = s.root(s.abspath(pathname), blogTrigger, true);

    boost::filesystem::path related = pathname;
    /* If *pathname* is not a regular file, we will build a list
       of posts related to the most recent post. */
    bool firstTime = true;
    boost::posix_time::ptime mostRecent;
    for( directory_iterator entry = directory_iterator(blogroot);
         entry != directory_iterator(); ++entry ) {
        boost::smatch m;
        path filename(*entry);
        if( is_regular_file(filename)
            && boost::regex_search(filename.string(),
                m,boost::regex(blogPat)) ) {
            boost::posix_time::ptime time
                = boost::posix_time::from_time_t(last_write_time(filename));
            if( firstTime || time > mostRecent ) {
                mostRecent = time;
                related = filename;
                firstTime = false;
            }
        }
    }
    return related;
}


void blogByIntervalTitle( session& s, const boost::filesystem::path& pathname )
{
	using namespace boost;

	smatch m;
	if( regex_search(pathname.string(),m,boost::regex(".*-(\\w+)$")) ) {
	    std::string name = m.str(1);
		s.out() << "Posts for " << name << std::endl;
	}
}


void blogEntryFetch( session& s, const boost::filesystem::path& pathname )
{
    /* This code is called through two different contexts. If we remove
	   the conditional assignment to s.feeds and write directly to s.out, 
	   two things happen:
	   - First, tags and other parsed attributes are discareded when called
	   within a feedContent() context. Meta-data are not read from within
	   the file but assigned to the default values set by feedContent() itself.
	   - Second, "by *author* on *date*" lines are either duplicated 
	   in the feedContent() context or not showing up in the stand-alone
	   context.
	*/

	htmlwriter feeds(s.out());
    if( !s.feeds ) {
		s.feeds = &feeds;
    }
    mailParser parser(boost::regex(blogPat),*s.feeds,true);
    parser.fetch(s,pathname);
    if( s.feeds == &feeds ) {
		s.feeds->flush();
		s.feeds = NULL;
    }
}


void mostRecentBlogTitle( session& s, const boost::filesystem::path& pathname )
{
	boost::filesystem::path entry = mostRecentBlogEntry(s,pathname);

    slice<char> text = s.loadtext(entry);
    mailAsPost listener;
    rfc2822Tokenizer tok(listener);
    tok.tokenize(text.begin(),text.size());
    post p = listener.unserialized();

	s.out() << p.title;
}


void mostRecentBlogFetch( session& s, const boost::filesystem::path& pathname )
{
	blogEntryFetch(s,mostRecentBlogEntry(s,pathname));
}
