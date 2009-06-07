#include <cassert>
#include <iostream>
#include "xmltok.hh"

const char *xmlTokenTitles[] = {
    "xmlErr",
    "xmlName",
    "xmlSpace",
    "xmlAssign",
    "xmlContent",
    "xmlEndDecl",
    "xmlAttValue",
    "xmlStartDecl"   
};


#define advance(state) { trans = &&state; goto advancePointer; }

bool nameStartChar( int c ) {
	return isalpha(c);
}

bool nameChar( int c ) {
	return isalnum(c);
}

void xmlTokenizer::tokenize( const char *line, size_t n )
{
    void *trans = state;
	const char *p = line;
	bool newline = false;
 
	if( trans != NULL ) goto *trans; else goto token;

advancePointer:
    ++p;
    switch( (std::distance(line,p) >= n) ? '\0' : *p ) {			     
    case '\r': 
		while( *p == '\r' ) ++p; 
		assert( *p == '\n' | *p == '\0' );
    case '\n':  
		newline = true;
    case '\0':  
		if( (p - line) - first > 0 && listener != NULL ) {
			listener->token(tok,line,first,p - line,false);
		}
		if( newline && listener != NULL ) listener->newline(); 
		state = NULL;
		return;	
    } 
    goto *trans;

attValueDouble:
	/* AttValue ::= '"' ([^<&"] | Reference)* '"' */
	if( *p != '"' ) advance(attValueDouble);
	advance(tag);

attValueSingle:
	/* AttValue ::= "'" ([^<&'] | Reference)* "'" */
	if( *p != '"' ) advance(attValueSingle);
	advance(tag);

 content:
	if( *p != '<' ) advance(content);
	goto token;

 endEmptyDecl:
	if( *p == '>' ) {
		tok = xmlEndDecl;
		advance(token);
	}
	goto error;

error:
    /* skip until tag delimiter character */
	if( *p != '<' & *p != '>' ) advance(error);
	goto tag;

name:
	if( nameChar(*p) ) advance(name);
	goto tag;	

spaces:
	if( *p == ' ' ) advance(spaces);
	goto tag;

startDecl:
	if( *p == '/' ) {
		tok = xmlEndDecl;
		advance(tag);
	}
	tok = xmlStartDecl;
	goto tag;

 tag:
    if( trans != NULL && listener != NULL ) {
		listener->token(tok,line,first,p - line,false);
		trans = NULL;
    }
    tok = xmlErr;
    first = p - line;
	if( nameStartChar(*p) ) {
		tok = xmlName;
		advance(name);
	}
	switch( *p ) {
	case ' ':
		tok = xmlSpace;
		advance(spaces);
	case '<':
		advance(startDecl);
	case '/':		
		advance(endEmptyDecl);
	case '>':
		tok = xmlEndDecl;
		advance(token);
	case '=':
		tok = xmlAssign;
		advance(tag);
	case '"':
		tok = xmlAttValue;
		advance(attValueDouble);
	case '\'':
		tok = xmlAttValue;
		advance(attValueSingle);
	}
	goto error;

token:
    if( trans != NULL && listener != NULL ) {
		listener->token(tok,line,first,p - line,false);
		trans = NULL;
    }
    tok = xmlErr;
    first = p - line;
	if( n == 0 || (*p == '\n' | *p == '\r' | *p == '\0') ) {
		--p;
		advance(token);
	}
	if( *p == '<' ) goto tag;
	tok = xmlContent;
	advance(content);
}
