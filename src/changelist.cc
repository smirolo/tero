/* Copyright (c) 2009, Sebastien Mirolo
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

   THIS SOFTWARE IS PROVIDED BY Sebastien Mirolo ''AS IS'' AND ANY
   EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL Sebastien Mirolo BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#include <cstdio>
#include "changelist.hh"
#include "markup.hh"

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
	std::cout << redirect(s.docAsUrl()) << '\n';
}


void change::fetch( session& s, const boost::filesystem::path& pathname ) {
    using namespace boost::system;
    using namespace boost::filesystem;

    session::variables::const_iterator document = s.vars.find("document");
    session::variables::const_iterator text = s.vars.find("editedText");
    if( text != s.vars.end() ) {
	path docName(s.vars["srcTop"] + document->second 
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
	file << text->second;
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
    std::cout << redirect(s.docAsUrl() + std::string(".edits")) << '\n';
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
	boost::filesystem::path 
	    gitrelname = relpath(docname,revision->rootpath);
	revision->diff(text,leftRevision,rightRevision,gitrelname);
		
	cout << "<table style=\"text-align: left;\">" << endl;
	cout << "<tr>" << endl;
	cout << "<th>" << leftRevision << "</th>" << endl;
	cout << "<th>" << rightRevision << "</th>" << endl;
	cout << "</tr>" << endl;

	boost::filesystem::ifstream input;
	open(input,docname);

	/* \todo the session is not a parameter to between files... */	
	document *doc = dispatchDoc::instance->select("document",docname.string());
	((::text*)doc)->showSideBySide(input,text,false);
		
	cout << "</table>" << endl;
	input.close();
    }
}


void 
changecheckin::fetch( session& s, const boost::filesystem::path& pathname ) 
{
    boost::filesystem::path sccsRoot = s.root(s.src(pathname),".git");
    if( !sccsRoot.empty() ) {
	checkinref ref;
	revision->rootPath(sccsRoot);
	revision->history(std::cout,s,pathname,ref);
    }
}

void 
changehistory::fetch( session& s, const boost::filesystem::path& pathname ) 
{
    boost::filesystem::path sccsRoot = s.root(s.src(pathname),".git");
    if( !sccsRoot.empty() ) {
	diffref ref;
	revision->rootPath(sccsRoot);
	revision->history(std::cout,s,pathname,ref);
    }
}


void 
changedescr::fetch( session& s, const boost::filesystem::path& pathname )
{
    boost::filesystem::path sccsRoot = s.root(s.src(pathname),".git");
    if( !sccsRoot.empty() ) {
	revision->rootPath(sccsRoot);

	history hist;
	revision->checkins(hist,s,pathname);

	/* Reference: http://www.rssboard.org/rss-specification */
	/* http://www.feedicons.com/ */
	htmlEscaper esc;

	std::cout << htmlContent << std::endl;

	for( history::checkinSet::const_iterator ci = hist.checkins.begin(); 
	     ci != hist.checkins.end(); ++ci ) {
	    std::cout << html::h(1) << ci->title << html::h(1).end();
	    std::cout << html::p();
	    std::cout <<  ci->time << " - " << ci->author;
	    std::cout << html::p::end;
	    std::cout << html::p();
	    esc.attach(std::cout);
	    std::cout << ci->descr;
	    esc.detach();
	    std::cout << html::p::end;
	    std::cout << html::p();
	    for( checkin::fileSet::const_iterator file = ci->files.begin(); 
		 file != ci->files.end(); ++file ) {
		std::cout << html::a().href(file->string()) << *file << html::a::end << "<br />" << std::endl;
	    }
	    std::cout << html::p::end;
	}	  
    }
}


void 
changerss::fetch( session& s, const boost::filesystem::path& pathname ) 
{
    boost::filesystem::path sccsRoot = s.root(s.src(pathname),".git");
    if( !sccsRoot.empty() ) {
	revision->rootPath(sccsRoot);

	history hist;
	revision->checkins(hist,s,pathname);

	/* Reference: http://www.rssboard.org/rss-specification */
	/* http://www.feedicons.com/ */
	htmlEscaper esc;

	/* \todo get the title and domainname from the session. */
	std::cout << ::rss().version("2.0")
		  << channel()
		  << rsslink()
		  << s.asUrl("") << rsslink::end
		  << title() << "Title" << title::end;

	for( history::checkinSet::const_iterator ci = hist.checkins.begin(); 
	     ci != hist.checkins.end(); ++ci ) {
	    std::cout << item();
	    std::cout << title() << ci->title << title::end;
	    std::cout << rsslink() << "domainname" << rsslink::end;
	    std::cout << guid() << ci->title << guid::end;
	    std::cout << author();
	    esc.attach(std::cout);
	    std::cout << ci->author;
	    esc.detach();
	    std::cout << author::end;
	    std::cout << pubDate() << ci->time << pubDate::end;
	    std::cout << description();
	    esc.attach(std::cout);
	    std::cout << ci->descr;
	    std::cout.flush();
	    esc.detach();
	    std::cout << description::end;
	    for( checkin::fileSet::const_iterator file = ci->files.begin(); 
		 file != ci->files.end(); ++file ) {
		std::cout << html::a().href(file->string()) << *file << html::a::end;
	    }
	    std::cout << item::end;
	}	
	std::cout << channel::end
		  << rss::end;	
    }
}
