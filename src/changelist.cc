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

#include <cstdio>
#include "changelist.hh"
#include "markup.hh"
#include "project.hh"
#include "revsys.hh"

/** Pages related to changes

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/

namespace {
    sessionVariable leftRev("left","commit tag for left pane of diff");
    sessionVariable rightRev("right","commit tag for right pane of diff");

} // anonymous namespace

pathVariable binDir("binDir","path to external executables");

void 
changelistAddSessionVars( boost::program_options::options_description& all,
			  boost::program_options::options_description& visible )
{
    using namespace boost::program_options;

    options_description localOptions("changelist");
    localOptions.add(binDir.option());
    localOptions.add(leftRev.option());
    localOptions.add(rightRev.option());
    all.add(localOptions);
    visible.add(localOptions);
}


url diffref::asUrl( const boost::filesystem::path& doc, 
		       const std::string& rev ) const {
    std::stringstream hrefs;
    hrefs << doc
	  << "/diff?right=" << rev; 
    return url(hrefs.str());
}

url checkinref::asUrl( const boost::filesystem::path& doc, 
		       const std::string& rev ) const {
    std::stringstream hrefs;
    hrefs << doc
	  << "/checkin?revision=" << rev; 
    return  url(hrefs.str());
}


void cancelFetch( session& s, const boost::filesystem::path& pathname ) {
    httpHeaders.location(url(document.value(s).string()));
}


void changeFetch(  session& s, const boost::filesystem::path& pathname )
{
    using namespace boost::system;
    using namespace boost::filesystem;

    session::variables::const_iterator text = s.find("editedText");
    if( s.found(text) ) {
	path docName
#if 0	
	/* \todo wiki-like edits are not currently working */
	(s.valueOf("srcTop") + s.valueOf("document")
		     + std::string(".edits"))
#endif
	;
	if( !exists(docName) ) {
	    create_directories(docName);
	}

	ofstream file;
	s.createfile(file,docName);
	file << text->second.value;
	file.close();

#if 0	
	/* \todo wiki-like edits are not currently working */
	/* add entry in the changelist */
	path changesPath(s.userPath().string() + std::string("/changes"));
	s.appendfile(changes,changesPath);
	file << docName;
	file.close();
#endif
    }
    httpHeaders.location(url(document.value(s).string() 
			     + std::string(".edits")));
}

void changediff( session& s, const boost::filesystem::path& pathname,
		 decorator *primary, decorator *secondary )
{
    using namespace std;

    std::stringstream text;
    std::string leftRevision = leftRev.value(s);
    std::string rightRevision = rightRev.value(s);
    boost::filesystem::path docname(pathname.parent_path());

    revisionsys *rev = revisionsys::findRev(s,docname);
    if( rev != NULL ) {
	boost::filesystem::path 
	    gitrelname = relpath(docname,rev->rootpath);
	rev->diff(text,leftRevision,rightRevision,gitrelname);
		
	s.out() << "<table style=\"text-align: left;\">" << endl;
	s.out() << html::tr();
	s.out() << html::th() << leftRevision << html::th::end;
	s.out() << html::th() << rightRevision << html::th::end;
	s.out() << html::tr::end;

	boost::filesystem::ifstream input;
	s.openfile(input,docname);

	/* \todo the session is not a parameter to between files... */	
	::text doc(*primary,*secondary);
	doc.showSideBySide(s,input,text,false);
	s.out() << html::table::end;
	input.close();
    }
    
}

void 
changecheckinFetch( session& s, const boost::filesystem::path& pathname )
{
    revisionsys *rev = revisionsys::findRev(s,pathname);
    if( rev ) {
	checkinref ref;
	rev->history(s.out(),s,pathname,ref);
    }
}

void 
changehistoryFetch( session& s, const boost::filesystem::path& pathname )
{
    revisionsys *rev = revisionsys::findRev(s,pathname);
    if( rev ) {
	diffref ref;
	rev->history(s.out(),s,pathname,ref);
    }
}


void changeShowDetails( session& s, const boost::filesystem::path& pathname ) {
    revisionsys *rev = revisionsys::findRev(s,pathname);
    if( rev ) {
	std::string commit = boost::filesystem::basename(pathname);
	htmlEscaper escaper;
	s.out() << code();
	escaper.attach(s.out());
	rev->showDetails(s.out(),commit);
	escaper.detach();
	s.out() << html::pre::end;
    }
}


void feedRepositoryPopulate( session& s, 
			    const boost::filesystem::path& pathname )
{
    revisionsys *rev = revisionsys::findRev(s,pathname);
    if( rev && s.feeds ) {
	history hist;
	boost::filesystem::path base =  boost::filesystem::path("/") 
	    / s.subdirpart(siteTop.value(s),rev->rootpath);
	boost::filesystem::path projname = projectName(s,rev->rootpath);	
	s.insert("title",projname.string());
	rev->checkins(s,pathname,*s.feeds);
#if 0
	for( history::checkinSet::iterator ci = hist.checkins.begin(); 
	     ci != hist.checkins.end(); ++ci ) {
	    ci->normalize();
	    std::stringstream strm;
	    strm << html::p();
	    strm << projname << " &nbsp;&mdash;&nbsp; ";
	    writelink(strm,base,ci->guid,".commit");
	    strm << "<br />";
	    strm << ci->content;
	    strm << html::p::end;
	    strm << html::pre();
	    for( checkin::fileSet::const_iterator file = ci->files.begin(); 
		 file != ci->files.end(); ++file ) {
		/* \todo link to diff with previous revision */
		writelink(strm,base,*file);
		strm << std::endl;
	    }
	    strm << html::pre::end;
	    post p(*ci);
	    p.content = strm.str();
	    s.feeds->filters(p);
	}	
#endif
    }
}


