#ifndef guardxmltok
#define guardxmltok

#include <iterator>

enum xmlToken {
    xmlErr,
    xmlName,
    xmlSpace,
    xmlAssign,
    xmlContent,
    xmlEndDecl,
    xmlAttValue,
    xmlStartDecl   
};

extern const char *xmlTokenTitles[];


/** Interface for callbacks from the xmlTokenizer
 */
class xmlTokListener {
public:
	xmlTokListener() {}

	virtual void newline() = 0;

	virtual void token( xmlToken token, const char *line, int first, int last, bool fragment ) = 0;
};


class xmlTokenizer {
protected:
	void *state;
	int first;
	xmlToken tok;
	int hexQuads;
	char expects;
	xmlTokListener *listener;

public:
    xmlTokenizer() 
	: state(NULL), first(0), tok(xmlErr), listener(NULL) {}

    xmlTokenizer( xmlTokListener& l ) 
	: state(NULL), first(0), tok(xmlErr), listener(&l) {}
    
    void attach( xmlTokListener& l ) { listener = &l; }

    void tokenize( const char *line, size_t n );
};


#endif
