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

#include <set>
#include <iostream>
#include "projfiles.hh"
#include "markup.hh"

/** Pages related to project files.

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


bool projfiles::selects( const boost::filesystem::path& pathname ) const {
    for( filterContainer::const_iterator filter = filters.begin();
	 filter != filters.end(); ++filter ) {
	if( boost::regex_match(pathname.string(),*filter) ) {
	    return true;
	}
    }
    return false;
}


void 
projfiles::addDir( session& s, const boost::filesystem::path& dir ) const {
    switch( state ) {
    case start:
	s.out() << html::div().classref("MenuWidget");
	break;
    case toplevelFiles:
    case direntryFiles:
	s.out() << html::p::end;
	break;	
    }
    state = direntryFiles;

    std::string href = dir.string();
    std::string srcTop = s.valueOf("srcTop");
    if( href.compare(0,srcTop.size(),srcTop) == 0 ) {
	href = s.root() + dir.string().substr(srcTop.size()) + "/dws.xml";
    }
    if( boost::filesystem::exists(dir.string() + "/dws.xml") ) {
	s.out() << html::a().href(href) 
		  << html::h(2)
		  << dir.leaf() 
		  << html::h(2).end()
		  << html::a::end;
    } else {
	s.out() << html::h(2) << dir.leaf() << html::h(2).end();
    }
    s.out() << html::p();
}


void 
projfiles::addFile( session& s, const boost::filesystem::path& file ) const {
    if( state == start ) {
	s.out() << html::div().classref("MenuWidget");
	s.out() << html::p();
	state = toplevelFiles;
    }
    s.out() << html::a().href(s.asUrl(file).string()) 
	      << file.leaf() 
	      << html::a::end << "<br />" << std::endl;
}


void projfiles::flush( session& s ) const 
{
    switch( state ) {
    case toplevelFiles:
    case direntryFiles:
	s.out() << html::p::end;
	s.out() << html::div::end;
	break;	
    default:
	/* Nothing to do excepts shutup gcc warnings. */
	break;
    }	
}


void projfiles::fetch( session& s, const boost::filesystem::path& pathname ) const
{
    using namespace std;
    using namespace boost::system;
    using namespace boost::filesystem;

    state = start;
    projdir = s.root(pathname,"dws.xml");
    
    if( !projdir.empty() ) {
	/* We insert pathnames into a set<> such that they later can be 
	   iterated in alphabetically sorted order. */
	std::set<path> topdirs;
	std::set<path> topfiles;
	for( directory_iterator entry = directory_iterator(projdir); 
		 entry != directory_iterator(); ++entry ) {
	    if( is_directory(*entry) ) {
		topdirs.insert(*entry);
	    } else {
		if( selects(*entry) ) topfiles.insert(*entry);
	    }
	}
	
	for( std::set<path>::const_iterator entry = topfiles.begin(); 
		 entry != topfiles.end(); ++entry ) {
	    addFile(s,*entry);
	}
	
	for( std::set<path>::const_iterator entry = topdirs.begin(); 
		 entry != topdirs.end(); ++entry ) {
	    std::set<path> files;
	    /* Insert all filename which match a filter */
	    for( directory_iterator file = directory_iterator(*entry); 
		 file != directory_iterator(); ++file ) {
		if( !is_directory(*file) ) {
		    if( selects(*file) ) files.insert(*file);
		}
	    }
	    if( !files.empty() ) {
		addDir(s,*entry);
		for( std::set<path>::const_iterator file = files.begin(); 
		     file != files.end(); ++file ) {
		    addFile(s,*file);
		}
	    }
	}
	flush(s);
    }
}
