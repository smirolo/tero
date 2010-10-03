/* Copyright (c) 2009, Fortylines LLC
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

#include <cstdio>
#include "changelist.hh"
#include "markup.hh"

/** Pages related to changes

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


url diffref::asUrl( const boost::filesystem::path& doc, 
		       const std::string& rev ) const {
    std::stringstream hrefs;
    hrefs << "/diff?document=" << doc
	  << "&right=" << rev; 
    return url(hrefs.str());
}

url checkinref::asUrl( const boost::filesystem::path& doc, 
		       const std::string& rev ) const {
    std::stringstream hrefs;
    hrefs << "/checkin?document=" << doc
	  << "&revision=" << rev; 
    return  url(hrefs.str());
}


void cancel::fetch( session& s, const boost::filesystem::path& pathname ) {
    *ostr << httpHeaders.location(url(s.doc())) << '\n';
}


void 
change::addSessionVars( boost::program_options::options_description& opts )
{
    using namespace boost::program_options;

    options_description changeOpts("changelist");
    changeOpts.add_options()    
	("href",value<std::string>(),"href")
	("right",value<std::string>(),"commit tag for right pane of diff")
	("editedText",value<std::string>(),"text submitted after an online edit");
    opts.add(changeOpts);
}


void change::fetch(  session& s, const boost::filesystem::path& pathname ) {
    using namespace boost::system;
    using namespace boost::filesystem;

    session::variables::const_iterator text = s.vars.find("editedText");
    if( text != s.vars.end() ) {
	path docName(s.valueOf("srcTop") + s.valueOf("document")
		     + std::string(".edits")); 

	if( !exists(docName) ) {
	    create_directories(docName);
	}

	ofstream file(docName);
	if( file.fail() ) {
	    boost::throw_exception(basic_filesystem_error<path>(
		std::string("unable to open file"),
		docName, 
		error_code()));
	}
	file << text->second.value;
	file.close();

	/* add entry in the changelist */
	path changesPath(s.userPath().string() + std::string("/changes"));
	ofstream changes(changesPath,std::ios::app);
	if( file.fail() ) {
	    boost::throw_exception(basic_filesystem_error<path>(
		std::string("unable to open file"),
		changesPath, 
		error_code()));
	}
	file << docName;
	file.close();
    }
    *ostr << httpHeaders.location(url(s.doc() + std::string(".edits")));
}


revisionsys::revsSet revisionsys::revs;

revisionsys*
revisionsys::findRev( session& s, const boost::filesystem::path& pathname ) {
    for( revsSet::iterator r = revs.begin(); r != revs.end(); ++r ) {
	boost::filesystem::path sccsRoot 
	    = s.root(s.src(pathname),(*r)->metadir);
	if( !sccsRoot.empty() ) {
	    (*r)->rootpath = sccsRoot;
	    return *r;
	}
    }
    return NULL;
}


void changediff::embed( session& s, const std::string& varname ) {
    using namespace std;

    if( varname != "document" ) {
	composer::embed(s,varname);
    } else {
	std::stringstream text;
	std::string leftRevision = s.valueOf("left");
	std::string rightRevision = s.valueOf("right");
	boost::filesystem::path docname(s.valueOf("srcTop") 
					+ s.valueOf(varname));

	revisionsys *rev = revisionsys::findRev(s,docname);
	if( rev != NULL ) {
	    boost::filesystem::path 
		gitrelname = relpath(docname,rev->rootpath);
	    rev->diff(text,leftRevision,rightRevision,gitrelname);
		
	    cout << "<table style=\"text-align: left;\">" << endl;
	    cout << html::tr();
	    cout << html::th() << leftRevision << html::th::end;
	    cout << html::th() << rightRevision << html::th::end;
	    cout << html::tr::end;

	    boost::filesystem::ifstream input;
	    open(input,docname);

	    /* \todo the session is not a parameter to between files... */	
	    document *doc = dispatchDoc::instance->select("document",docname.string());
	    ((::text*)doc)->showSideBySide(input,text,false);
		
	    cout << html::table::end;
	    input.close();
	}
    }
}


void 
changecheckin::fetch( session& s, const boost::filesystem::path& pathname ) 
{
    revisionsys *rev = revisionsys::findRev(s,pathname);
    if( rev ) {
	checkinref ref;
	rev->history(*ostr,s,pathname,ref);
    }
}

void 
changehistory::fetch( session& s, const boost::filesystem::path& pathname ) 
{
    revisionsys *rev = revisionsys::findRev(s,pathname);
    if( rev ) {
	diffref ref;
	rev->history(*ostr,s,pathname,ref);
    }
}


void 
changedescr::fetch( session& s, const boost::filesystem::path& pathname )
{
    using namespace std;

    revisionsys *rev = revisionsys::findRev(s,pathname);
    if( rev ) {
	history hist;
	rev->checkins(hist,s,pathname);

	/* Reference: http://www.rssboard.org/rss-specification */
	/* RSS Icons: http://www.feedicons.com/ */
	htmlEscaper esc;

#if 1
	for( history::checkinSet::const_iterator ci = hist.checkins.begin(); 
	     ci != hist.checkins.end(); ++ci ) {
	    *ostr << html::h(2) << ci->title << html::h(2).end();
	    *ostr << html::p();
	    *ostr <<  ci->time << " - " << ci->authorEmail;
	    *ostr << html::p::end;
	    *ostr << html::p();
	    esc.attach(*ostr);
	    *ostr << ci->descr;
	    esc.detach();
	    *ostr << html::p::end;
	    *ostr << html::p();
	    for( checkin::fileSet::const_iterator file = ci->files.begin(); 
		 file != ci->files.end(); ++file ) {
		*ostr << html::a().href(file->string()) << *file << html::a::end << "<br />" << std::endl;
	    }
	    *ostr << html::p::end;
	}
#else
	htmlwriter liner(*ostr);
	for( history::checkinSet::const_iterator ci = hist.checkins.begin(); 
	     ci != hist.checkins.end(); ++ci ) {
	    liner.filters(*ci);
	}	
#endif	  
    }
}


void 
changerss::fetch( session& s, const boost::filesystem::path& pathname ) 
{
    revisionsys *rev = revisionsys::findRev(s,pathname);
    if( rev ) {
	history hist;
	rev->checkins(hist,s,pathname);

	/* Reference: http://www.rssboard.org/rss-specification */
	/* http://www.feedicons.com/ */
	htmlEscaper esc;

	/* \todo get the title and domainname from the session. */
	*ostr << httpHeaders.contentType("application/rss+xml");

	*ostr << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << std::endl;	
#if 0
	*ostr << ::rss().version("2.0")
		  << channel()
		  << title() 
		  << boost::filesystem::basename(rev->rootpath)
		  << title::end;
		  << rsslink() << rsslink::end
#else
	*ostr << "<rss version='2.0' xmlns:lj='http://www.livejournal.org/rss/lj/1.0/' xmlns:atom=\"http://www.w3.org/2005/Atom\">" << std::endl;
	*ostr << channel();
	*ostr << title() 
		  << boost::filesystem::basename(rev->rootpath)
		  << title::end;
	*ostr << "<description></description>\n";
	*ostr << rsslink()
		  << rsslink::end;
 
	*ostr << "<atom:link href=\"index.rss\" rel=\"self\" type=\"application/rss+xml\" />" << std::endl;
	
#endif

	for( history::checkinSet::const_iterator ci = hist.checkins.begin(); 
	     ci != hist.checkins.end(); ++ci ) {
	    *ostr << item();
	    *ostr << title() << ci->title << title::end;
#if 0
	    *ostr << rsslink() << rsslink::end;
#endif
	    *ostr << description();
	    esc.attach(*ostr);
	    *ostr << html::p();
	    *ostr << ci->descr;
	    *ostr << html::pre();
	    for( checkin::fileSet::const_iterator file = ci->files.begin(); 
		 file != ci->files.end(); ++file ) {
		*ostr << html::a().href(file->string()) 
			  << *file << html::a::end << std::endl;
	    }
	    *ostr << html::pre::end;
	    *ostr << html::p::end;
	    ostr->flush();
	    esc.detach();
	    *ostr << description::end;
	    *ostr << author();
	    esc.attach(*ostr);
	    *ostr << ci->authorEmail;
	    esc.detach();
	    *ostr << author::end;
#if 0
	    *ostr << "<guid isPermaLink=\"true\">";
#endif
	    *ostr << guid() << ci->guid << ".html" << guid::end;
	    *ostr << pubDate(ci->time);
	    *ostr << item::end;
	}	
	*ostr << channel::end << rss::end;	
    }
}
