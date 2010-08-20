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

#include <iostream>
#include <boost/regex.hpp>
#include <boost/program_options.hpp>
#include <boost/system/error_code.hpp>
#include "document.hh"
#include "markup.hh"


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


void document::meta( session& s, const boost::filesystem::path& pathname ) {
    if( s.vars.find("title") == s.vars.end() ) {
	if( s.vars.find("Subject") != s.vars.end() ) {
	    s.vars["title"] = s.valueOf("Subject");
	} else {
	    s.vars["title"] = s.valueOf("document");
	}
    }
}


dispatchDoc *dispatchDoc::instance = NULL;


dispatchDoc::dispatchDoc( const boost::filesystem::path& t ) : root(t) {
    instance = this;
}

void dispatchDoc::add( const std::string& varname, 
		    const boost::regex& r, 
		    document& d ) {
    presentationSet::iterator aliases = views.find(varname);
    if( aliases == views.end() ) {
	/* first pattern we are adding for the variable */
	views[varname] = aliasSet();
	aliases = views.find(varname);
    }
    aliases->second.push_back(std::make_pair(r,&d));
}


void dispatchDoc::fetch( session& s, const std::string& varname ) {
    document* doc = select(varname,s.valueOf(varname));
    if( doc != NULL ) {
	doc->fetch(s,s.valueAsPath(varname));
    }
}


document* dispatchDoc::select( const std::string& name, 
			    const std::string& value ) const {
    presentationSet::const_iterator view = views.find(name);
    if( view != views.end() ) {
	const aliasSet& aliases = view->second;
	for( aliasSet::const_iterator alias = aliases.begin(); 
	     alias != aliases.end(); ++alias ) {
	    if( regex_match(value,alias->first) ) {
		return alias->second;
	    }
	}
    }
    return NULL;
}


void dirwalker::fetch( session& s, const boost::filesystem::path& pathname )
{
    using namespace boost::filesystem;

    first();
    if( is_directory(pathname) ) {
	for( directory_iterator entry = directory_iterator(pathname); 
	     entry != directory_iterator(); ++entry ) {
	    if( !is_directory(*entry) ) {
		boost::filesystem::ifstream infile;
		open(infile,*entry);
		walk(s,infile,entry->string());
		infile.close();
	    }
	}
    } else {
	boost::filesystem::ifstream infile;
	open(infile,pathname);
	walk(s,infile);
	infile.close();
    }
    last();
}


void text::showSideBySide( std::istream& input,
			   std::istream& diff,
			   bool inputIsLeftSide ) {
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
		cout << "<tr class=\"diffConflict\">" << endl;
		cout << html::td();
		if( leftDec->formated() ) cout << code();
		cout << left.str();
		if( nbLeftLinesAhead < nbRightLinesAhead ) {
		    for( int i = 0; i < (nbRightLinesAhead - nbLeftLinesAhead); ++i ) { 
			cout << std::endl;
		    }
		} 
		if( leftDec->formated() ) cout << html::pre::end;
		cout << html::td::end;
		    
		cout << html::td();
		if( rightDec->formated() ) cout << code();
		cout << right.str();
		if(  nbLeftLinesAhead > nbRightLinesAhead ) {
		    for( int i = 0; i < (nbLeftLinesAhead - nbRightLinesAhead); ++i ) { 
			cout << endl;
		    }
		}
		if( rightDec->formated() ) cout << html::pre::end;
		cout << html::td::end;
		cout << html::tr::end;
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
		cout << html::tr();
		cout << html::td();
		if( leftDec->formated() ) cout << code();
		cout << left.str();
		if( leftDec->formated() ) cout << html::pre::end;
		cout << html::td::end;
		cout << html::td();
		if( rightDec->formated() ) cout << code();
		cout << right.str();
		if( rightDec->formated() ) cout << html::pre::end;
		cout << html::td::end;
		cout << html::tr::end;
		left.str("");
		right.str("");
	    }
	    ++nbRightLinesAhead;
	    areDiffBlocks = true;
	    right << line.substr(1) << endl;
		
	} else if( line[0] == leftMarker ) {
	    if( !areDiffBlocks & !left.str().empty() ) {
		cout << html::tr();
		cout << html::td();
		if( leftDec->formated() ) cout << code();
		cout << left.str();
		if( leftDec->formated() ) cout << html::pre::end;
		cout << html::td::end;
		cout << html::td();
		if( rightDec->formated() ) cout << code();
		cout << right.str();
		if( rightDec->formated() ) cout << html::pre::end;
		cout << html::td::end;
		cout << html::tr::end;
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
		cout << "<tr class=\"diff" 
		     << ((!left.str().empty() & !right.str().empty()) ? "" : "No") 
		     << "Conflict\">" << endl;
		cout << html::td();
		if( leftDec->formated() ) cout << code();
		cout << left.str();
		if( nbLeftLinesAhead < nbRightLinesAhead ) {
		    for( int i = 0; i < (nbRightLinesAhead - nbLeftLinesAhead); ++i ) { 
			cout << std::endl;
		    }
		} 
		if( leftDec->formated() ) cout << html::pre::end;
		cout << html::td::end;
		    
		cout << html::td();
		if( rightDec->formated() ) cout << code();
		cout << right.str();
		if(  nbLeftLinesAhead > nbRightLinesAhead ) {
		    for( int i = 0; i < (nbLeftLinesAhead - nbRightLinesAhead); ++i ) { 
			cout << endl;
		    }
		}
		if( rightDec->formated() ) cout << html::pre::end;
		cout << html::td::end;
		cout << html::tr::end;
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
	    cout << "<tr class=\"diff" 
		 << ((!left.str().empty() & !right.str().empty()) ? "" : "No") 
		 << "Conflict\">" << endl;
	} else {
	    cout << html::tr();
	}
	cout << html::td();
	if( leftDec->formated() ) cout << code();
	cout << left.str();
	if( nbLeftLinesAhead < nbRightLinesAhead ) {
	    for( int i = 0; i < (nbRightLinesAhead - nbLeftLinesAhead); ++i ) { 
		cout << std::endl;
	    }
	} 
	if( leftDec->formated() ) cout << html::pre::end;
	cout << html::td::end;
	cout << html::td();
	if( rightDec->formated() ) cout << code();
	cout << right.str();
	if(  nbLeftLinesAhead > nbRightLinesAhead ) {
	    for( int i = 0; i < (nbLeftLinesAhead - nbRightLinesAhead); ++i ) { 
		cout << endl;
	    }
	}
	if( rightDec->formated() ) cout << html::pre::end;
	cout << html::td::end;
	cout << html::tr::end;
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
	cout << html::tr();
	cout << html::td();
	if( leftDec->formated() ) cout << code();	
	cout << left.str();
	if( leftDec->formated() ) cout << html::pre::end;
	cout << html::td::end;
	cout << html::td();
	if( rightDec->formated() ) cout << code();	
	cout << right.str();
	if( rightDec->formated() ) cout << html::pre::end;
	cout << html::td::end;
	cout << html::tr::end;
    }

    leftDec->detach();
    rightDec->detach();
}


void text::meta( session& s, const boost::filesystem::path& pathname ) {
    using namespace boost::filesystem; 
    static const boost::regex valueEx("^(\\S+):\\s+(.*)");

    ifstream strm;
    open(strm,pathname);
    while( !strm.eof() ) {
	boost::smatch m;
	std::string line;
	std::getline(strm,line);
	if( boost::regex_search(line,m,valueEx) ) {
	    s.vars[m.str(1)] = m.str(2);
	} else break;
    }
    strm.close();
    document::meta(s,pathname);

    /* 
    std::time_t last_write_time( const path & ph );
    To convert the returned value to UTC or local time, 
    use std::gmtime() or std::localtime() respectively. */
}


void text::fetch( session& s, const boost::filesystem::path& pathname ) {
    using namespace boost;
    using namespace boost::system;
    using namespace boost::filesystem; 

    ifstream strm;
    open(strm,pathname);

    *ostr << httpHeaders;

    if( leftDec ) {
	if( leftDec->formated() ) *ostr << code();
	leftDec->attach(*ostr);
    }

    skipOverTags(strm);

    /* remaining lines */
    while( !strm.eof() ) {
	std::string line;
	std::getline(strm,line);
	*ostr << line << std::endl;
    }

    if( leftDec ) {
	leftDec->detach();
	if( leftDec->formated() ) *ostr << html::pre::end;
    }

    strm.close();
}


void text::skipOverTags( std::istream& istr )
{
    static const boost::regex valueEx("^(\\S+):\\s+(.*)");

    /* Skip over tags */
    while( !istr.eof() ) {
	boost::smatch m;
	std::string line;
	std::getline(istr,line);
	if( !boost::regex_search(line,m,valueEx) ) {	
	    *ostr << line << std::endl;
	    break;
	}
    }
}
