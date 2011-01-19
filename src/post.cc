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

#include "mail.hh"
#include "markup.hh"
#include <boost/date_time/date_facet.hpp>

/** Posts.

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


shortPost::shortPost( const post& p, const std::string& t ) 
  : postBase(p), tag(t)
{
}

post::post( const post& p, const std::string& t ) 
    : postBase(p), tag(t), guid(p.guid), descr(p.descr), score(p.score)
{
}


void 
post::addSessionVars( boost::program_options::options_description& opts )
{
    using namespace boost::program_options;

    options_description postOptions("posts");
    postOptions.add_options()
	("title",value<std::string>(),"title")
	("author",value<std::string>(),"author")
	("descr",value<std::string>(),"descr");
    opts.add(postOptions);
}


void post::normalize() {
    title = ::normalize(title);
    authorName = ::normalize(authorName);
    authorEmail = ::normalize(authorEmail);
    guid = ::strip(guid);
    descr = ::strip(descr);
}


bool post::valid() const {
    /* If there are any whitespaces in the guid, Thunderbird 3 will
       hang verifying the rss feed... */
    for( size_t i = 0; i < guid.size(); ++i ) {
	if( isspace(guid[i]) ) {
	    return false;
	}
    }    
    return (!title.empty() & !authorEmail.empty() & !descr.empty());
}


void postFilter::filters( const post& p ) {
    if( next ) next->filters(p);
}

void postFilter::flush() {
    if( next ) next->flush();
}


void htmlwriter::filters( const post& p ) {
    using namespace boost::gregorian;
    using namespace boost::posix_time;

    time_facet* facet(new time_facet(pubDate::format));
    (*ostr).imbue(std::locale((*ostr).getloc(), facet));

    *ostr << html::div().classref( (postNum % 2 == 0) ? 
				   "postEven" : "postOdd");

    /* caption for the post */
    *ostr << html::div().classref("postCaption");    
    *ostr << "by " << html::a().href(std::string("mailto:") + p.authorEmail)
	  << p.authorName << html::a::end
	  << " on " << p.time;
    *ostr << html::div::end;    

    /* body of the post */
    *ostr << html::div().classref("postBody");    
    *ostr << p.descr;
    *ostr << html::div::end;
    
    *ostr << html::div::end;    
    ++postNum;
}


void blogwriter::filters( const post& p ) {
    using namespace boost::gregorian;
    using namespace boost::posix_time;

    time_facet* facet(new time_facet(pubDate::format));
    (*ostr).imbue(std::locale((*ostr).getloc(), facet));

    *ostr << "From " << p.authorEmail << std::endl;
    if( !p.title.empty() ) {
	*ostr << "Subject: " << p.title << std::endl;
    }
    *ostr << "Date: " << p.time << std::endl;
    *ostr << "From: " << p.authorEmail << std::endl;    

    *ostr << "Score: " << p.score << std::endl;
    *ostr << std::endl << std::endl;
    /* \todo avoid description starting with "From " */
    *ostr << p.descr << std::endl << std::endl;
}


void rsswriter::filters( const post& p ) {
    htmlEscaper esc;

    *ostr << item();
    *ostr << title() << p.title << title::end;

    *ostr << description();
    esc.attach(*ostr);
    *ostr << html::p() << p.authorName << ":" << html::p::end;
    *ostr << p.descr;
    ostr->flush();
    esc.detach();
    *ostr << description::end;

    *ostr << author();
    esc.attach(*ostr);
    *ostr << p.authorEmail;
    esc.detach();
    *ostr << author::end;

#if 0
    *ostr << "<guid isPermaLink=\"true\">";
#endif
    *ostr << guid() << p.guid << guid::end;
    *ostr << pubDate(p.time);
    *ostr << item::end;
}
