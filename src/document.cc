#include <iostream>
#include <boost/regex.hpp>
#include <boost/program_options.hpp>
#include <boost/system/error_code.hpp>
#include "document.hh"

void document::open( boost::filesystem::ifstream& strm, 
				 const boost::filesystem::path& pathname ) const {
	using namespace boost::system;
	using namespace boost::filesystem;
	strm.open(pathname);
	if( strm.fail() ) {
		boost::throw_exception(basic_filesystem_error<path>(std::string("file not found"),
															pathname, 
															error_code()));
#if 0
		error_code(posix_error::no_such_file_or_directory)));
#endif
	}
}


dispatch *dispatch::instance = NULL;


dispatch::dispatch( const boost::filesystem::path& t ) : root(t) {
    instance = this;
}

void dispatch::add( const std::string& varname, const boost::regex& r, document& d ) {
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


document* dispatch::select( const std::string& name, const std::string& value ) const {
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
				cout << "<td>" << endl;
				cout << left.str();
				if( nbLeftLinesAhead < nbRightLinesAhead ) {
					for( int i = 0; i < (nbRightLinesAhead - nbLeftLinesAhead); ++i ) { 
						cout << std::endl;
					}
				} 
				cout << "</td>" << endl;
				
				cout << "<td>" << endl;
				cout << right.str();
				if(  nbLeftLinesAhead > nbRightLinesAhead ) {
					for( int i = 0; i < (nbLeftLinesAhead - nbRightLinesAhead); ++i ) { 
						cout << endl;
					}
				}
				cout << "</td>" << endl;
				cout << "</tr>" << endl;
				left.str("");
				right.str("");
			}

			char *p = strchr(line.c_str(),leftMarker);
			size_t start = atoi(&p[1]);
			/* read left file until we hit the start line */
			while( leftLine < start ) {
				std::string l;
				getline(input,l);
				*leftDec << l;
				*rightDec << l;
				++leftLine;
			}
			nbLeftLinesAhead = nbRightLinesAhead = 0;
			areDiffBlocks = false;

		} else if( line[0] == rightMarker ) {
			if( !areDiffBlocks & !left.str().empty() ) {
				cout << "<tr>" << endl;
				cout << "<td>" << endl;
				cout << left.str();
				cout << "</td>" << endl;
				cout << "<td>" << endl;
				cout << right.str();
				cout << "</td>" << endl;
				cout << "</tr>" << endl;
				left.str("");
				right.str("");
			}
			++nbRightLinesAhead;
			areDiffBlocks = true;
			*rightDec << line.substr(1);

		} else if( line[0] == leftMarker ) {
			if( !areDiffBlocks & !left.str().empty() ) {
				cout << "<tr>" << endl;
				cout << "<td>" << endl;
				cout << left.str();
				cout << "</td>" << endl;
				cout << "<td>" << endl;
				cout << right.str();
				cout << "</td>" << endl;
				cout << "</tr>" << endl;
				left.str("");
				right.str("");
			}
			std::string l;
			getline(input,l);
			++nbLeftLinesAhead;
			areDiffBlocks = true;
			*leftDec << l;
			++leftLine;

		} else if( line[0] == ' ' ) {
			if( areDiffBlocks ) {
				cout << "<tr class=\"diff" 
					 << ((!left.str().empty() & !right.str().empty()) ? "" : "No") 
					 << "Conflict\">" << endl;
				cout << "<td>" << endl;
				cout << left.str();
				if( nbLeftLinesAhead < nbRightLinesAhead ) {
					for( int i = 0; i < (nbRightLinesAhead - nbLeftLinesAhead); ++i ) { 
						cout << std::endl;
					}
				} 
				cout << "</td>" << endl;
				
				cout << "<td>" << endl;
				cout << right.str();
				if(  nbLeftLinesAhead > nbRightLinesAhead ) {
					for( int i = 0; i < (nbLeftLinesAhead - nbRightLinesAhead); ++i ) { 
						cout << endl;
					}
				}
				cout << "</td>" << endl;
				cout << "</tr>" << endl;
				left.str("");
				right.str("");
				areDiffBlocks = false;
				nbLeftLinesAhead = nbRightLinesAhead = 0;
			}
			std::string l;
			getline(input,l);
			*leftDec << l;
			++leftLine;
			*rightDec << line.substr(1);
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
		cout << left.str();
		if( nbLeftLinesAhead < nbRightLinesAhead ) {
			for( int i = 0; i < (nbRightLinesAhead - nbLeftLinesAhead); ++i ) { 
				cout << std::endl;
			}
		} 
		cout << "</td>" << endl;
		cout << "<td>" << endl;
		cout << right.str();
		if(  nbLeftLinesAhead > nbRightLinesAhead ) {
			for( int i = 0; i < (nbLeftLinesAhead - nbRightLinesAhead); ++i ) { 
				cout << endl;
			}
		}
		cout << "</td>" << endl;
		cout << "</tr>" << endl;
		left.str("");
		right.str("");
	}
}


void text::fetch( session& s, const boost::filesystem::path& pathname ) {
	using namespace boost;
    using namespace boost::system;
    using namespace boost::filesystem; 

    ifstream strm;
    open(strm,pathname);
	std::cout << htmlContent;

#if 1
	leftDec->attach(std::cout);
#endif
    while( !strm.eof() ) {
		std::string line;
		std::getline(strm,line);
		std::cout << line << std::endl;
    }
#if 2
	leftDec->detach();
#endif
    strm.close();
}


