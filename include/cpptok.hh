#ifndef guardcpptok
#define guardcpptok

#include <iterator>

enum cppToken {
    cppErr,
    cppBooleanLiteral,
    cppCharacterLiteral,
    cppFloatingLiteral,
    cppDecimalLiteral,
    cppOctalLiteral,
    cppHexadecimalLiteral,
    cppIdentifier,
    cppKeyword,
    cppOperator,
    cppPunctuator,
    cppComment,
    cppSpace,
    cppTabSpace,
    cppPreprocessing,
    cppStringLiteral
};

extern const char *cppTokenTitles[];


/** Interface for callbacks from the cppTokenizer
 */
class cppTokListener {
public:
	cppTokListener() {}

	virtual void newline() = 0;

	virtual void token( cppToken token, const char *line, int first, int last, bool fragment ) = 0;
};


class xmlCppTokListener : public cppTokListener {
protected:
	std::ostream *ostr;
	
public:
	explicit xmlCppTokListener( std::ostream& o ) : ostr(&o) {}
	
	void token( cppToken token, const char *line, int first, int last, bool fragment ) {
		*ostr << '<' << cppTokenTitles[token];
		if( fragment ) *ostr << " fragment=\"" << fragment << "\"";
		*ostr << " text=\"[" << first << "," << last << "]\">";
		std::copy(&line[first],&line[last],std::ostream_iterator<char>(*ostr));
		*ostr << "</" << cppTokenTitles[token] << ">";
	}
};


template<typename charT, typename traitsT>
class htmlCppTokListener : public cppTokListener {
protected:
	std::basic_ostream<charT,traitsT> *ostr;
	
public:
	explicit htmlCppTokListener( std::basic_ostream<charT,traitsT>& o ) : ostr(&o) {}
	
public:
	void token( cppToken token, const char *line, int first, int last, bool fragment ) {
		*ostr << "<span class=\"" << cppTokenTitles[token] << "\">";
		std::copy(&line[first],&line[last],std::ostream_iterator<charT>(*ostr));
		*ostr << "</span>";
	}

	void endl() {
		*ostr << std::endl;
	}
};


class cppTokenizer {
protected:
	void *state;
	int first;
	cppToken tok;
	int hexQuads;
	char expects;
	cppTokListener *listener;

public:
    cppTokenizer() 
	: state(NULL), first(0), tok(cppErr), listener(NULL) {}

    explicit cppTokenizer( cppTokListener& l ) 
	: state(NULL), first(0), tok(cppErr), listener(&l) {}
    
    void attach( cppTokListener& l ) { listener = &l; }
    
    void tokenize( const char *line, size_t n );
};


#endif
