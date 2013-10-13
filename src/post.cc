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

#include "mail.hh"
#include "markup.hh"
#include <boost/date_time/date_facet.hpp>
#include "decorator.hh"
#include "contrib.hh"

/** Posts.

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/

sessionVariable titleVar("title","title of a post");
sessionVariable authorVar("author","author of a post");
sessionVariable authorEmail("authorEmail","email for the author of a post");
sessionVariable descrVar("descr","content of a post");


void 
postAddSessionVars( boost::program_options::options_description& opts,
		      boost::program_options::options_description& visible )
{
    using namespace boost::program_options;

    options_description localOptions("posts");
    localOptions.add(titleVar.option());
    localOptions.add(authorVar.option());
    localOptions.add(authorEmail.option());
    localOptions.add(descrVar.option());
    opts.add(localOptions);
    visible.add(localOptions);
}


void post::normalize() {
    title = ::normalize(title);
    guid = ::strip(guid);
    content = ::strip(content);
}


bool post::valid() const {
    /* If there are any whitespaces in the guid, Thunderbird 3 will
       hang verifying the rss feed... */
    for( size_t i = 0; i < guid.size(); ++i ) {
        if( isspace(guid[i]) ) {
            return false;
        }
    }
    return (!title.empty() & !author->email.empty() & !content.empty());
}


void passThruFilter::flush() {
    if( next ) next->flush();
}


void retainedFilter::provide() {
    first = posts.begin();
    last = posts.end();
}


void retainedFilter::flush()
{
    if( next ) {
	provide();
	for( const_iterator p = first; p != last; ++p ) {
	    next->filters(*p);
	}
	next->flush();
    }
}


void contentHtmlwriter::filters( const post& p ) {
    *ostr << p.content;
}


void htmlwriter::filters( const post& p ) {
    using namespace boost::gregorian;
    using namespace boost::posix_time;

    time_facet* facet(new time_facet(pubDate::shortFormat));
    (*ostr).imbue(std::locale((*ostr).getloc(), facet));

    *ostr << html::div().classref( (postNum % 2 == 0) ?
        "postEven" : "postOdd");

    /* caption for the post */
    *ostr << html::div().classref("postCaption");
    if( !p.link.empty() ) {
        *ostr << p.link << std::endl;

    } else if( !notitle && !p.title.empty() ) {
        *ostr << html::a().href(p.guid)
              << html::h(1) << p.title << html::h(1).end()
              << html::a::end << std::endl;
    }
    *ostr << by(p.author) << " on " << p.time;
    *ostr << html::div::end;

    /* body of the post */
    *ostr << html::div().classref("postBody");
    contentHtmlwriter::filters(p);
    *ostr << html::div::end;

    *ostr << html::div().classref("postContact");
    *ostr << html::div().classref("contactAuthor");
    *ostr << "Contact the author "
          << html::div() << contact(p.author) << html::div::end;
    *ostr << html::div::end;

    /* social sharing */
    *ostr << html::div();
    *ostr << "Share with your network";
    *ostr << html::div().classref("shareNetwork");
    *ostr << "<div data-social-share-privacy='true'></div>" << std::endl;
    *ostr << html::div::end;
    *ostr << html::div::end;
    *ostr << "<br />" << std::endl;
    *ostr << html::div::end; // postContact

    *ostr << html::div::end;
    ++postNum;
}


void mailwriter::filters( const post& p ) {
    using namespace boost::gregorian;
    using namespace boost::posix_time;

    time_facet* facet(new time_facet(pubDate::format));
    (*ostr).imbue(std::locale((*ostr).getloc(), facet));

    *ostr << "From " << from(p.author) << std::endl;
	*ostr << "Subject: " << (!p.title.empty() ? p.title : "(No Subject)")
		  << std::endl;
    *ostr << "Date: " << p.time << std::endl;
    *ostr << "From: " << from(p.author) << std::endl;
    *ostr << "Score: " << p.score << std::endl;

    for( post::headersMap::const_iterator header = p.moreHeaders.begin();
	 header != p.moreHeaders.end(); ++header ) {
		*ostr << header->first << ": " << header->second << std::endl;
    }

    *ostr << std::endl << std::endl;
	if( !p.link.empty() ) {
		*ostr << p.link << std::endl;
	}
    /* \todo avoid description starting with "From " */
    *ostr << p.content << std::endl << std::endl;
}


void rsswriter::filters( const post& p ) {
    htmlEscaper esc;

    *ostr << item();
    *ostr << title() << p.title << title::end;
    *ostr << rsslink() << p.guid << rsslink::end;

    *ostr << description() << "<![CDATA[";
	if( !p.link.empty() ) {
		*ostr << p.link << std::endl;
	}
    *ostr << html::p()
		  << by(p.author) << ":" << "<br />"
		  << p.content << html::p::end
		  << "]]>" << description::end;

    *ostr << author();
    esc.attach(*ostr);
    *ostr << p.author->email;
    esc.detach();
    *ostr << author::end;

#if 0
    *ostr << "<guid isPermaLink=\"true\">";
#endif
    *ostr << guid() << p.guid << guid::end;
    *ostr << pubDate(p.time);
    *ostr << item::end;
}


void subjectWriter::filters( const post& p ) {
    *ostr << html::a().href(p.guid) << p.title << html::a::end 
		  << "<br/>" << std::endl;
}
