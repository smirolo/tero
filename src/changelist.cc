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


std::ostream& checkin::content( std::ostream& ostr ) const
{
    ostr << html::p();
    post::content(ostr);
    ostr << html::p::end;
    ostr << html::pre();
    for( checkin::fileSet::const_iterator file = files.begin(); 
	 file != files.end(); ++file ) {
	ostr << html::a().href(file->string()) 
	      << *file << html::a::end << std::endl;
    }
    ostr << html::pre::end;
    return ostr;
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




