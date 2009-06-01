#include <string>
#include <fstream>
#include <iostream>


class cmpFile {
protected:
    char *name;
    std::istream *istr;
    std::ostream *ostr;

public:
    size_t line;

    cmpFile( char* n ) : line(1), name(n) {}

    void filter( std::istream& i, std::ostream& o ) { istr = &i; ostr = &o; } 

    void nextLine() {
	std::cerr << name << ":nextLine" << std::endl;
	char l[256]; 
	istr->getline(l,256);
	*ostr << l << std::endl;
	++line;
    }

    void blankLines( size_t n ) {
	std::cerr << name << ":blankLines(" << n << ")" << std::endl;
	for( int i = 0; i < n; ++i ) { 
	    *ostr << std::endl;
	}
    }

};


bool startswith( const char *str, const char* s ) {
    return strncmp(str,s,strlen(s)) == 0;
}

char *asFilename( char *str ) {
    char *p = str;
    while( *p != '\t' ) ++p;
    *p = '\0';
    return str;
}

int main( int argc, char *argv[] )
{
    using namespace std;

    if( argc < 2 ) {
	cerr << "usage: " << argv[0] << " diffname" << endl;
    }

    char line[256]; 
    ifstream diff(argv[1]);
    ifstream leftInput, rightInput;
    ofstream leftOutput, rightOutput;
    cmpFile left("left"), right("right");
    int nbLeftLinesAhead, nbRightLinesAhead;

    while( !diff.eof() ) {
	diff.getline(line,256);
	std::cerr << "diff:" << line << std::endl;
	if( startswith(line,"---") ) {
	    leftInput.open(asFilename(&line[4]));
	    leftOutput.open("result1.txt");
	    left.filter(leftInput,leftOutput);

	} else if( startswith(line,"+++") ) {
	    rightInput.open(asFilename(&line[4]));
	    rightOutput.open("result2.txt");
	    right.filter(rightInput,rightOutput);
	    
	} else if( startswith(line,"@@") ) {
	    size_t start = atoi(&line[4]);
	    /* read left file until we hit the start line */
	    while( left.line < start ) {
		left.nextLine();
	    }
	    std::cerr << "@@ -" << start << std::endl;
	    char *p = ::strchr(line,'+');
	    start = atoi(p);
	    std::cerr << "@@ +" << start << std::endl;
	    /* read right file until we hit the start line */
	    while( right.line < start ) {
		right.nextLine();
	    }
	    nbLeftLinesAhead = nbRightLinesAhead = 0;
	} else if( startswith(line,"+") ) {
	    ++nbRightLinesAhead;
	    right.nextLine();
	} else if( startswith(line,"-") ) {
	    ++nbLeftLinesAhead;
	    left.nextLine();
	} else if( startswith(line," ") ) {
	    if( nbLeftLinesAhead < nbRightLinesAhead ) {
		left.blankLines(nbRightLinesAhead - nbLeftLinesAhead);
	    } else if(  nbLeftLinesAhead > nbRightLinesAhead ) {
		right.blankLines(nbLeftLinesAhead - nbRightLinesAhead);
	    }
	    nbLeftLinesAhead = nbRightLinesAhead = 0;
	    left.nextLine();
	    right.nextLine();
	}
    }

    leftInput.close();
    rightInput.close();
    leftOutput.close();
    rightOutput.close();
    diff.close();
    return 0;
}
