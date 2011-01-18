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

/** Base document classes

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


void document::open( boost::filesystem::ifstream& strm, 
		     const boost::filesystem::path& pathname ) const {
    using namespace boost::system;
    using namespace boost::filesystem;
    if( is_regular_file(pathname) ) {
	strm.open(pathname);
	if( !strm.fail() ) return;
    }
    /* \todo figure out how to pass iostream error code in exception. */
    boost::throw_exception(
        basic_filesystem_error<path>(std::string("file not found"),
				     pathname, 
				     error_code()));
}


void createfile( boost::filesystem::ofstream& strm, 
		 const boost::filesystem::path& pathname ) {
    using namespace boost::system;
    using namespace boost::filesystem;

    strm.open(pathname,std::ios_base::out | std::ios_base::trunc);
    if( strm.fail() ) {
	boost::throw_exception(basic_filesystem_error<path>(
	   std::string("unable to create file"),
	   pathname, 
	   error_code()));
    }
}


void document::meta( session& s, const boost::filesystem::path& pathname ) const {
    if( s.vars.find("title") == s.vars.end() ) {
	if( s.vars.find("Subject") != s.vars.end() ) {
	    s.insert("title",s.valueOf("Subject"));
	} else {
	    s.insert("title",s.valueOf("document"));
	}
    }
}


dispatchDoc *dispatchDoc::instance = NULL;


dispatchDoc::dispatchDoc( const boost::filesystem::path& t ) : root(t) {
    instance = this;
}

void dispatchDoc::add( const std::string& varname, 
		    const boost::regex& r, 
		    const document& d ) {
    presentationSet::iterator aliases = views.find(varname);
    if( aliases == views.end() ) {
	/* first pattern we are adding for the variable */
	views[varname] = aliasSet();
	aliases = views.find(varname);
    }
    aliases->second.push_back(std::make_pair(r,&d));
}


void dispatchDoc::fetch( session& s, const std::string& varname ) {
    const document* doc = select(varname,s.valueOf(varname));
    if( doc != NULL ) {
	doc->fetch(s,s.valueAsPath(varname));
    }
}


const document* 
dispatchDoc::select( const std::string& name, const std::string& value ) const {
    presentationSet::const_iterator view = views.find(name);
    if( view != views.end() ) {
	const aliasSet& aliases = view->second;
	for( aliasSet::const_iterator alias = aliases.begin(); 
	     alias != aliases.end(); ++alias ) {
	    boost::smatch m;
#if 0
	    std::cerr << "match(" << value << ", " << alias->first << ")?" << std::endl;
#endif
	    if( regex_match(value,m,alias->first) ) {
#if 0
		std::cerr << "select(" << value << ") = ";
		for( boost::smatch::const_iterator p = m.begin();
		     p != m.end(); ++p ) {
		    std::cerr << ", " << *p;
		}
		std::cerr << std::endl;
#endif		
		return alias->second;
	    }
	}
    }
    return NULL;
}


void dirwalker::fetch( session& s, const boost::filesystem::path& pathname ) const
{
    using namespace boost::filesystem;

    first();
    if( is_directory(pathname) ) {
	for( directory_iterator entry = directory_iterator(pathname); 
	     entry != directory_iterator(); ++entry ) {
	    boost::smatch m;
	    if( !is_directory(*entry) 
		&& boost::regex_match(entry->string(),m,filematch) ) {		
		boost::filesystem::ifstream infile;
		open(infile,*entry);
		walk(s,infile,entry->string());
		infile.close();
	    }
	}
    } else {

	std::cerr << "!!! yep!" << std::endl;

	boost::filesystem::ifstream infile;
	open(infile,pathname);
	walk(s,infile);
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
    int nbLeftLinesAhead, nbRightLinesAhead;
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


void text::meta( session& s, const boost::filesystem::path& pathname ) const {
    using namespace boost::filesystem; 
    static const boost::regex valueEx("^(\\S+):\\s+(.*)");

    ifstream strm;
    open(strm,pathname);
    while( !strm.eof() ) {
	boost::smatch m;
	std::string line;
	std::getline(strm,line);
	if( boost::regex_search(line,m,valueEx) ) {
	    s.insert(m.str(1),m.str(2));
	} else break;
    }
    strm.close();
    document::meta(s,pathname);

    /* 
    std::time_t last_write_time( const path & ph );
    To convert the returned value to UTC or local time, 
    use std::gmtime() or std::localtime() respectively. */
}


void text::fetch( session& s, const boost::filesystem::path& pathname ) const {
    using namespace boost;
    using namespace boost::system;
    using namespace boost::filesystem; 

    ifstream strm;
    open(strm,pathname);

    if( !header.empty() ) s.out() << header;

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


void text::skipOverTags( session& s, std::istream& istr ) const
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
