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

/** Pages related to changes

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


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
    httpHeaders.location(url(s.doc()));
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


void changeFetch(  session& s, const boost::filesystem::path& pathname )
{
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
    httpHeaders.location(url(s.doc() + std::string(".edits")));
}

void changediff( session& s, const boost::filesystem::path& pathname,
		 decorator *primary, decorator *secondary )
{
    using namespace std;

    std::stringstream text;
    std::string leftRevision = s.valueOf("left");
    std::string rightRevision = s.valueOf("right");
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
	openfile(input,docname);

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




