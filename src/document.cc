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

#include <iostream>
#include <boost/regex.hpp>
#include <boost/program_options.hpp>
#include <boost/system/error_code.hpp>
#include "document.hh"
#include "markup.hh"
#include <sys/stat.h>
#include <pwd.h>
#include "decorator.hh"

/** Base document functions

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/

dispatchDoc *dispatchDoc::singleton = NULL;


dispatchDoc::dispatchDoc( fetchEntry* e, size_t n )
    : entries(e), nbEntries(n) {
    singleton = this;
    fetchEntry* prev = NULL;
    for( fetchEntry* first = entries; first != &entries[nbEntries]; ++first ) {
	if( prev ) {
	    if( strcmp(prev->name,first->name) > 0  ) {
		std::cerr << "entries are not sorted (" << prev->name 
			  << " and " << first->name << ")" << std::endl;
	    }
	}
	prev = first;
    }
}


bool dispatchDoc::fetch( session& s, 
			 const std::string& name,
			 const url& value ) {    
    using namespace boost::filesystem;

    const fetchEntry *doc = select(name,value.string());
    if( doc != NULL ) {
	path p(s.abspath(value));
#if 0	
	std::cerr << "behavior: " << doc->behavior << " " << p << std::endl;
#endif
	path dir = p;
	path prev = current_path();
	while( !is_directory(dir) ) {
	    /* boost is returning root/log as the parent path 
	       for root/log/ (note the trailing backslash). 
	       Of course it isn't what we expected here. */
	    dir.remove_leaf();
	}
	current_path(dir);
	switch( doc->behavior ) {
	case whenFileExist:
	    s.check(p);
	case always:
	    doc->callback(s,p);
	    break;
	case whenNotCached:
	    doc->callback(s,p);
	    break;
	}	
	current_path(prev);
    }
    return ( doc != NULL );
}




const fetchEntry* 
dispatchDoc::select( const std::string& name, const std::string& value ) const {
    fetchEntry cmp;
    cmp.name = name.c_str();
    fetchEntry *lastEntry = &entries[nbEntries];
    fetchEntry *first = std::lower_bound(entries,lastEntry,cmp);
    fetchEntry *last = std::upper_bound(entries,lastEntry,cmp);	
    if( first != lastEntry ) {
	for( fetchEntry *start = first; start != last; ++start ) {
	    boost::smatch m;
	    if( regex_match(value,m,start->pat) ) {
#if 0
		std::cerr << "select(\"" << name << "\"," << value 
			  << ") matches " << start->pat << std::endl;
#endif		
		return start;
	    }
	}
    }
#if 1
    std::cerr << "select(\"" << name << "\"," << value 
	      << ") does not match any of " << std::endl;
    for( fetchEntry *start = first; start != last; ++start ) {
	std::cerr << "- (" << value << "," << start->pat << ")" << std::endl;
    }
#endif
    return NULL;
}


void dirwalker::fetch( session& s, const boost::filesystem::path& pathname )
{
    using namespace boost::filesystem;

    first();
    if( is_directory(pathname) ) {
	for( directory_iterator entry = directory_iterator(pathname); 
	     entry != directory_iterator(); ++entry ) {
	    boost::smatch m;
	    path p = *entry;
	    if( !is_directory(*entry) 
		&& boost::regex_match(p.string(),m,filematch) ) {	
		boost::filesystem::ifstream infile;
		s.openfile(infile,*entry);
		walk(s,infile,p.string());
		infile.close();
	    }
	}
    } else {
	boost::filesystem::ifstream infile;
	s.openfile(infile,pathname);
	walk(s,infile,pathname.string());
	infile.close();
    }
    last();
}


void text::showSideBySide( session& s, 
			   std::istream& input,
			   std::istream& diff,
			   bool inputIsLeftSide ) const {
    using namespace std;
    
    std::stringstream left, right;
    size_t leftLine = 1;
    bool areDiffBlocks = false;
    int nbLeftLinesAhead = 0, nbRightLinesAhead = 0;
    char leftMarker = inputIsLeftSide ? '-' : '+';
    char rightMarker = inputIsLeftSide ? '+' : '-';
    
    leftDec->attach(left);
    rightDec->attach(right);
    while( !diff.eof() ) {
	std::string line;
	getline(diff,line);
	if( line.compare(0,3,"+++") == 0
	    || line.compare(0,3,"---") == 0 ) {
		
	} else if( line.compare(0,2,"@@") == 0 ) {
	    if( areDiffBlocks ) {
		s.out() << "<tr class=\"diffConflict\">" << endl;
		s.out() << html::td();
		if( leftDec->formated() ) s.out() << code();
		s.out() << left.str();
		if( nbLeftLinesAhead < nbRightLinesAhead ) {
		    for( int i = 0; i < (nbRightLinesAhead - nbLeftLinesAhead); ++i ) { 
			s.out() << std::endl;
		    }
		} 
		if( leftDec->formated() ) s.out() << html::pre::end;
		s.out() << html::td::end;
		    
		s.out() << html::td();
		if( rightDec->formated() ) s.out() << code();
		s.out() << right.str();
		if(  nbLeftLinesAhead > nbRightLinesAhead ) {
		    for( int i = 0; i < (nbLeftLinesAhead - nbRightLinesAhead); ++i ) { 
			s.out() << endl;
		    }
		}
		if( rightDec->formated() ) s.out() << html::pre::end;
		s.out() << html::td::end;
		s.out() << html::tr::end;
		left.str("");
		right.str("");
	    }
		
	    const char *p = strchr(line.c_str(),leftMarker);
	    size_t start = atoi(&p[1]);
	    /* read left file until we hit the start line */
	    while( leftLine < start ) {
		std::string l;
		getline(input,l);
		left << l << endl;
		right << l << endl;
		++leftLine;
	    }
	    nbLeftLinesAhead = nbRightLinesAhead = 0;
	    areDiffBlocks = false;
		
	} else if( line[0] == rightMarker ) {
	    if( !areDiffBlocks & !left.str().empty() ) {
		s.out() << html::tr();
		s.out() << html::td();
		if( leftDec->formated() ) s.out() << code();
		s.out() << left.str();
		if( leftDec->formated() ) s.out() << html::pre::end;
		s.out() << html::td::end;
		s.out() << html::td();
		if( rightDec->formated() ) s.out() << code();
		s.out() << right.str();
		if( rightDec->formated() ) s.out() << html::pre::end;
		s.out() << html::td::end;
		s.out() << html::tr::end;
		left.str("");
		right.str("");
	    }
	    ++nbRightLinesAhead;
	    areDiffBlocks = true;
	    right << line.substr(1) << endl;
		
	} else if( line[0] == leftMarker ) {
	    if( !areDiffBlocks & !left.str().empty() ) {
		s.out() << html::tr();
		s.out() << html::td();
		if( leftDec->formated() ) s.out() << code();
		s.out() << left.str();
		if( leftDec->formated() ) s.out() << html::pre::end;
		s.out() << html::td::end;
		s.out() << html::td();
		if( rightDec->formated() ) s.out() << code();
		s.out() << right.str();
		if( rightDec->formated() ) s.out() << html::pre::end;
		s.out() << html::td::end;
		s.out() << html::tr::end;
		left.str("");
		right.str("");
	    }
	    std::string l;
	    getline(input,l);
	    ++nbLeftLinesAhead;
	    areDiffBlocks = true;
	    left << l << endl;
	    ++leftLine;
		
	} else if( line[0] == ' ' ) {
	    if( areDiffBlocks ) {
		s.out() << "<tr class=\"diff" 
		     << ((!left.str().empty() & !right.str().empty()) ? "" : "No") 
		     << "Conflict\">" << endl;
		s.out() << html::td();
		if( leftDec->formated() ) s.out() << code();
		s.out() << left.str();
		if( nbLeftLinesAhead < nbRightLinesAhead ) {
		    for( int i = 0; i < (nbRightLinesAhead - nbLeftLinesAhead); ++i ) { 
			s.out() << std::endl;
		    }
		} 
		if( leftDec->formated() ) s.out() << html::pre::end;
		s.out() << html::td::end;
		    
		s.out() << html::td();
		if( rightDec->formated() ) s.out() << code();
		s.out() << right.str();
		if(  nbLeftLinesAhead > nbRightLinesAhead ) {
		    for( int i = 0; i < (nbLeftLinesAhead - nbRightLinesAhead); ++i ) { 
			s.out() << endl;
		    }
		}
		if( rightDec->formated() ) s.out() << html::pre::end;
		s.out() << html::td::end;
		s.out() << html::tr::end;
		left.str("");
		right.str("");
		areDiffBlocks = false;
		nbLeftLinesAhead = nbRightLinesAhead = 0;
	    }
	    std::string l;
	    getline(input,l);
	    left << l << endl;
	    ++leftLine;
	    right << line.substr(1) << endl;
	}
    }
    if( !left.str().empty() | !right.str().empty() ) {
	if( areDiffBlocks ) {
	    s.out() << "<tr class=\"diff" 
		 << ((!left.str().empty() & !right.str().empty()) ? "" : "No") 
		 << "Conflict\">" << endl;
	} else {
	    s.out() << html::tr();
	}
	s.out() << html::td();
	if( leftDec->formated() ) s.out() << code();
	s.out() << left.str();
	if( nbLeftLinesAhead < nbRightLinesAhead ) {
	    for( int i = 0; i < (nbRightLinesAhead - nbLeftLinesAhead); ++i ) { 
		s.out() << std::endl;
	    }
	} 
	if( leftDec->formated() ) s.out() << html::pre::end;
	s.out() << html::td::end;
	s.out() << html::td();
	if( rightDec->formated() ) s.out() << code();
	s.out() << right.str();
	if(  nbLeftLinesAhead > nbRightLinesAhead ) {
	    for( int i = 0; i < (nbLeftLinesAhead - nbRightLinesAhead); ++i ) { 
		s.out() << endl;
	    }
	}
	if( rightDec->formated() ) s.out() << html::pre::end;
	s.out() << html::td::end;
	s.out() << html::tr::end;
	left.str("");
	right.str("");
    }
    if( !input.eof() ) {
	while( !input.eof() ) {
	    std::string line;
	    getline(input,line);
	    left << line << endl;
	    right << line << endl;
	}
	s.out() << html::tr();
	s.out() << html::td();
	if( leftDec->formated() ) s.out() << code();	
	s.out() << left.str();
	if( leftDec->formated() ) s.out() << html::pre::end;
	s.out() << html::td::end;
	s.out() << html::td();
	if( rightDec->formated() ) s.out() << code();	
	s.out() << right.str();
	if( rightDec->formated() ) s.out() << html::pre::end;
	s.out() << html::td::end;
	s.out() << html::tr::end;
    }

    leftDec->detach();
    rightDec->detach();
}


void skipOverTags( session& s, std::istream& istr )
{
    static const boost::regex valueEx("^(\\S+):\\s+(.*)");

    /* Skip over tags */
    while( !istr.eof() ) {
	boost::smatch m;
	std::string line;
	std::getline(istr,line);
	if( !boost::regex_search(line,m,valueEx) ) {	
	    s.out() << line << std::endl;
	    break;
	}
    }
}


void text::fetch( session& s, const boost::filesystem::path& pathname ) {
    using namespace boost;
    using namespace boost::system;
    using namespace boost::filesystem; 

    ifstream strm;
    s.openfile(strm,pathname);

    if( leftDec ) {
	if( leftDec->formated() ) s.out() << code();
	leftDec->attach(s.out());
    }

    skipOverTags(s,strm);

    /* remaining lines */
    while( !strm.eof() ) {
	std::string line;
	std::getline(strm,line);
	s.out() << line << std::endl;
    }

    if( leftDec ) {
	leftDec->detach();
	if( leftDec->formated() ) s.out() << html::pre::end;
    }

    strm.close();
}


void textFetch( session& s, const boost::filesystem::path& pathname ) {
    htmlEscaper leftLinkText;
    htmlEscaper rightLinkText;
    text rawtext(leftLinkText,rightLinkText);
    rawtext.fetch(s,pathname);
}


void formattedFetch( session& s, const boost::filesystem::path& pathname ) {
    linkLight leftFormatedText(s);
    linkLight rightFormatedText(s);
    text formatedText(leftFormatedText,rightFormatedText);
    formatedText.fetch(s,pathname);
}

void metaLastTime( session& s, const boost::filesystem::path& pathname ) {    
    std::time_t lwt = last_write_time(pathname);
    boost::posix_time::ptime time = boost::posix_time::from_time_t(lwt);
    std::stringstream strm;
    mbox_string(strm,time);
    s.insert("time",strm.str());
    s.out() << strm.str();
}

void metaValue( session& s, const boost::filesystem::path& pathname )
{
    s.out() << pathname;
}


void metaFileOwner( session& s, const boost::filesystem::path& pathname ) {
    using namespace boost::filesystem;

    std::string author("anonymous");
    std::string authorEmail("info");
    struct stat statbuf;
    if( stat(pathname.string().c_str(),&statbuf) == 0 ) {
	struct passwd *pwd = getpwuid(statbuf.st_uid);
	if( pwd != NULL ) {
	    authorEmail =pwd->pw_name;
	    author = pwd->pw_gecos;
	}
    }
    authorEmail += std::string("@") + domainName.value(s).string();
    s.insert("author",author);
    s.insert("authorEmail",authorEmail);
    s.out() << author;
}
