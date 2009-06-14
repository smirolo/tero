/* Copyright (c) 2009, Sebastien Mirolo
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of codespin nor the
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

boost::filesystem::path 
document::relativePath( const boost::filesystem::path& pathname,
			const boost::filesystem::path& base ) {
    boost::filesystem::path::iterator first = pathname.begin();
    boost::filesystem::path::iterator second = base.begin();
    boost::filesystem::path result;
    
    for( ; first != pathname.end() & second != base.end(); 
		 ++first, ++second ) {
		if( *first != *second ) break;
    }
    for( ; first != pathname.end(); ++first ) {
		result /= *first;
    }
    return result;
}


boost::filesystem::path document::root( const session& s,
					const boost::filesystem::path& leaf,
					const boost::filesystem::path& trigger )
{
    using namespace boost::filesystem;
    std::string topSrc = s.vars.find("topSrc")->second;
    path dirname = leaf;
    if( !is_directory(dirname) ) {
	dirname.remove_leaf();
    }
    bool foundProject = exists(dirname.string() / trigger);
    while( !foundProject & dirname.string() != topSrc ) {
	std::cerr << "look for " << trigger << " into " << dirname << std::endl;
	dirname.remove_leaf();
	if( dirname.string().empty() ) {
	    boost::throw_exception(basic_filesystem_error<path>(
			std::string("no trigger from path up"),
			leaf, 
			boost::system::error_code())); 
	}
	foundProject = exists(dirname.string() / trigger);
    }
    return foundProject ? dirname : path("");
}

void document::open( boost::filesystem::ifstream& strm, 
		     const boost::filesystem::path& pathname ) const {
    using namespace boost::system;
    using namespace boost::filesystem;
    if( is_regular_file(pathname) ) {
	strm.open(pathname);
	if( !strm.fail() ) return;
    }
    boost::throw_exception(
        basic_filesystem_error<path>(std::string("file not found"),
				     pathname, 
				     error_code()));
#if 0
    error_code(posix_error::no_such_file_or_directory)));
#endif
}


dispatch *dispatch::instance = NULL;


dispatch::dispatch( const boost::filesystem::path& t ) : root(t) {
    instance = this;
}

void dispatch::add( const std::string& varname, const boost::regex& r, 
					document& d ) {
    presentationSet::iterator aliases = views.find(varname);
    if( aliases == views.end() ) {
		/* first pattern we are adding for the variable */
		views[varname] = aliasSet();
		aliases = views.find(varname);
    }
    aliases->second.push_back(std::make_pair(r,&d));
}


void dispatch::fetch( session& s, const std::string& varname ) {
	document* doc = select(varname,s.vars[varname]);
	if( doc != NULL ) {
		doc->fetch(s,boost::filesystem::path(root.string() + s.vars[varname]));
	}
}


document* dispatch::select( const std::string& name, 
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

#if 1
	leftDec->attach(left);
	rightDec->attach(right);
#endif
    while( !diff.eof() ) {
		std::string line;
		getline(diff,line);
		if( line.compare(0,3,"+++") == 0
			|| line.compare(0,3,"---") == 0 ) {
			
		} else if( line.compare(0,2,"@@") == 0 ) {
			if( areDiffBlocks ) {
				cout << "<tr class=\"diffConflict\">" << endl;
				cout << "<td>" << endl;
				if( leftDec->formated() ) cout << "<pre>" << endl;
				cout << left.str();
				if( nbLeftLinesAhead < nbRightLinesAhead ) {
					for( int i = 0; i < (nbRightLinesAhead - nbLeftLinesAhead); ++i ) { 
						cout << std::endl;
					}
				} 
				if( leftDec->formated() ) cout << "</pre>" << endl;
				cout << "</td>" << endl;
				
				cout << "<td>" << endl;
				if( rightDec->formated() ) cout << "<pre>" << endl;
				cout << right.str();
				if(  nbLeftLinesAhead > nbRightLinesAhead ) {
					for( int i = 0; i < (nbLeftLinesAhead - nbRightLinesAhead); ++i ) { 
						cout << endl;
					}
				}
				if( rightDec->formated() ) cout << "</pre>" << endl;
				cout << "</td>" << endl;
				cout << "</tr>" << endl;
				left.str("");
				right.str("");
			}

			char *p = strchr(line.c_str(),leftMarker);
			size_t start = atoi(&p[1]);
			/* read left file until we hit the start line */
			std::cerr << "read " << (start - leftLine)  << " lines from input file." << std::endl; 
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
				cout << "<tr>" << endl;
				cout << "<td>" << endl;
				if( leftDec->formated() ) cout << "<pre>" << endl;
				cout << left.str();
				if( leftDec->formated() ) cout << "</pre>" << endl;
				cout << "</td>" << endl;
				cout << "<td>" << endl;
				if( rightDec->formated() ) cout << "<pre>" << endl;
				cout << right.str();
				if( rightDec->formated() ) cout << "</pre>" << endl;
				cout << "</td>" << endl;
				cout << "</tr>" << endl;
				left.str("");
				right.str("");
			}
			++nbRightLinesAhead;
			areDiffBlocks = true;
			right << line.substr(1) << endl;

		} else if( line[0] == leftMarker ) {
			if( !areDiffBlocks & !left.str().empty() ) {
				cout << "<tr>" << endl;
				cout << "<td>" << endl;
				if( leftDec->formated() ) cout << "<pre>" << endl;
				cout << left.str();
				if( leftDec->formated() ) cout << "</pre>" << endl;
				cout << "</td>" << endl;
				cout << "<td>" << endl;
				if( rightDec->formated() ) cout << "<pre>" << endl;
				cout << right.str();
				if( rightDec->formated() ) cout << "</pre>" << endl;
				cout << "</td>" << endl;
				cout << "</tr>" << endl;
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
				cout << "<td>" << endl;
				if( leftDec->formated() ) cout << "<pre>" << endl;
				cout << left.str();
				if( nbLeftLinesAhead < nbRightLinesAhead ) {
					for( int i = 0; i < (nbRightLinesAhead - nbLeftLinesAhead); ++i ) { 
						cout << std::endl;
					}
				} 
				if( leftDec->formated() ) cout << "</pre>" << endl;
				cout << "</td>" << endl;
				
				cout << "<td>" << endl;
				if( rightDec->formated() ) cout << "<pre>" << endl;
				cout << right.str();
				if(  nbLeftLinesAhead > nbRightLinesAhead ) {
					for( int i = 0; i < (nbLeftLinesAhead - nbRightLinesAhead); ++i ) { 
						cout << endl;
					}
				}
				if( rightDec->formated() ) cout << "</pre>" << endl;
				cout << "</td>" << endl;
				cout << "</tr>" << endl;
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
			cout << "<tr>" << endl;
		}
		cout << "<td>" << endl;
		if( leftDec->formated() ) cout << "<pre>" << endl;
		cout << left.str();
		if( nbLeftLinesAhead < nbRightLinesAhead ) {
			for( int i = 0; i < (nbRightLinesAhead - nbLeftLinesAhead); ++i ) { 
				cout << std::endl;
			}
		} 
		if( leftDec->formated() ) cout << "</pre>" << endl;
		cout << "</td>" << endl;
		cout << "<td>" << endl;
		if( rightDec->formated() ) cout << "<pre>" << endl;
		cout << right.str();
		if(  nbLeftLinesAhead > nbRightLinesAhead ) {
			for( int i = 0; i < (nbLeftLinesAhead - nbRightLinesAhead); ++i ) { 
				cout << endl;
			}
		}
		if( rightDec->formated() ) cout << "</pre>" << endl;
		cout << "</td>" << endl;
		cout << "</tr>" << endl;
		left.str("");
		right.str("");
	}
#if 1
	leftDec->detach();
	rightDec->detach();
#endif
}


void text::fetch( session& s, const boost::filesystem::path& pathname ) {
	using namespace boost;
    using namespace boost::system;
    using namespace boost::filesystem; 

    ifstream strm;
    open(strm,pathname);
	std::cout << htmlContent;

#if 1
	if( leftDec->formated() ) std::cout << "<pre>" << std::endl;
	leftDec->attach(std::cout);
#endif
    while( !strm.eof() ) {
		std::string line;
		std::getline(strm,line);
		std::cout << line << std::endl;
    }
#if 1
	leftDec->detach();
	if( leftDec->formated() ) std::cout << "</pre>" << std::endl;
#endif
    strm.close();
}


