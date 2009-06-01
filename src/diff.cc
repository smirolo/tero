#include <sstream>
#include <iostream>
#include <boost/filesystem/fstream.hpp>
#include "diff.hh"

void diff::showSideBySide( document *doc,
						   std::istream& input,
						   std::istream& diff,
						   bool inputIsLeftSide ) {
	using namespace std;
	assert( doc != NULL );

	size_t leftLine = 1;
	stringstream left, right;
	bool areDiffBlocks = false;
    int nbLeftLinesAhead, nbRightLinesAhead;
	char leftMarker = inputIsLeftSide ? '-' : '+';
	char rightMarker = inputIsLeftSide ? '+' : '-';
	document::decorator* leftBlock = doc->decorate(left);
	document::decorator* rightBlock = doc->decorate(right);

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
				leftBlock->write(l);
				rightBlock->write(l);
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
			rightBlock->write(line.substr(1));

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
			leftBlock->write(l);
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
			leftBlock->write(l);
			++leftLine;
			rightBlock->write(line.substr(1));
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
	if( leftBlock != NULL ) delete leftBlock;
	if( rightBlock != NULL ) delete rightBlock;

}

void diff::betweenFiles( const boost::filesystem::path& leftPath, 
						 const boost::filesystem::path& rightPath ) {

	using namespace std;
	using namespace boost::filesystem;

	stringstream cmd;
	/* The diff parser expects a unified diff format */
	cmd << "diff -u " << leftPath << ' ' << rightPath;
	FILE *diffFile = popen(cmd.str().c_str(),"r");
	if( diffFile == NULL ) {
		std::cerr << "error opening command: " << cmd.str() << std::endl;
		return;
	}
	const int maxLineSize = 255;
	char line[maxLineSize];
	stringstream diff;
	while( fgets(line,maxLineSize,diffFile) != NULL ) {
		std::cerr << line;
		diff << line;
	}
	pclose(diffFile);

	cout << "<table>" << endl;
	cout << "<tr>" << endl;
	cout << "<td>" << leftPath << "</td>" << endl;
	cout << "<td>" << rightPath << "</td>" << endl;
	cout << "</tr>" << endl;

    boost::filesystem::ifstream input(leftPath);

	/* \todo the session is not a parameter to between files... */
	document *doc = dispatch::instance->select("document",leftPath.string());
	showSideBySide(doc,input,diff);

	cout << "</table>" << endl;
    input.close();
}


void diff::fetch( session& s, const boost::filesystem::path& pathname ) 
{
	using namespace std;

	const std::string& refname = s.valueOf("ref");
	betweenFiles(refname,pathname);
}
