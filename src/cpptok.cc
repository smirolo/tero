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

#include <cstdlib>
#include <cassert>
#include <string>
#include <algorithm>
#include <iostream>
#include <boost/regex.hpp>
#include "cpptok.hh"

const char *cppTokenTitles[] = {
    "err",
    "booleanLiteral",
    "characterLiteral",
    "floatingLiteral",
    "decimalLiteral",
    "octalLiteral",
    "hexadecimalLiteral",
    "identifier",
    "unstyledIdentifier",
    "keyword",
    "operator",
    "punctuator",
    "comment",
    "space",
    "tab",
    "preprocessing",
    "stringLiteral"
};

struct idenTokenType {
	std::string identifier;
	cppToken token;
};

bool operator<( const idenTokenType& left, const idenTokenType& right ) {
	return left.identifier < right.identifier;
}

idenTokenType idenTokenClass[] = {
	{ "asm", cppKeyword },
	{ "auto", cppKeyword }, 
	{ "bool", cppKeyword }, 
	{ "break", cppKeyword }, 
	{ "case", cppKeyword }, 
	{ "catch", cppKeyword }, 
	{ "char", cppKeyword }, 
	{ "class", cppKeyword }, 
	{ "const", cppKeyword }, 
	{ "const_cast", cppKeyword }, 
	{ "continue", cppKeyword }, 
	{ "default", cppKeyword }, 
	{ "delete", cppKeyword },
	{ "do", cppKeyword },
	{ "double", cppKeyword },
	{ "dynamic_cast", cppKeyword },
	{ "else", cppKeyword },
	{ "enum", cppKeyword },
	{ "explicit", cppKeyword },
	{ "export", cppKeyword },
	{ "extern", cppKeyword },
	{ "false", cppBooleanLiteral },
	{ "float", cppKeyword },
	{ "for", cppKeyword },
	{ "friend", cppKeyword },
	{ "goto", cppKeyword },
	{ "inline", cppKeyword },
	{ "int", cppKeyword },
	{ "long", cppKeyword },
	{ "mutable", cppKeyword },
	{ "namespace", cppKeyword },
	{ "new", cppKeyword },
	{ "operator", cppKeyword },
	{ "private", cppKeyword },
	{ "protected", cppKeyword },
	{ "public", cppKeyword },
	{ "register", cppKeyword },
	{ "reinterpret_cast", cppKeyword },
	{ "return", cppKeyword },
	{ "short", cppKeyword },
	{ "signed", cppKeyword },
	{ "sizeof", cppKeyword },
	{ "static", cppKeyword },
	{ "static_assert", cppKeyword },
	{ "static_cast", cppKeyword },
	{ "struct", cppKeyword },
	{ "switch", cppKeyword },
	{ "template", cppKeyword },
	{ "this", cppKeyword },
	{ "throw", cppKeyword },
	{ "true", cppBooleanLiteral },
	{ "try", cppKeyword }, 
	{ "typedef", cppKeyword },
	{ "typeid", cppKeyword },
	{ "typename", cppKeyword },
	{ "union", cppKeyword },
	{ "unsigned", cppKeyword },
	{ "using", cppKeyword },
	{ "virtual", cppKeyword },
	{ "void", cppKeyword },
	{ "volatile", cppKeyword },
	{ "wchar_t", cppKeyword },
	{ "while", cppKeyword }, 
};

cppToken identifierToken( const std::string& str ) {
    idenTokenType look;
    look.identifier = str;
    idenTokenType *last 
	= &idenTokenClass[sizeof(idenTokenClass)/sizeof(idenTokenType)];
    idenTokenType *found = std::lower_bound(idenTokenClass,last,look);
    if( found != last && found->identifier == look.identifier ) {
	return found->token;
    }
    /* If the identifier does not match accepted style (here camelCase
       starting with a lowercase), mark it with a slightly different tag. */
    static const boost::regex pat("[a-z,0-9]+([A-Z][a-z,0-9]+)*");
    if( boost::regex_match(str,pat) ) {
	return cppIdentifier;
    }
    return cppIncorrectIdentifier;
}


bool isSeparator( int c ) {
    switch( c ) {
    case ' ':
    case '\t':
    case '\n':
    case '\'':
    case '"':
    case '{':
    case '}':
    case '[':
    case ']':
    case '(':
    case ')':
    case ';':
    case ',':
    case '#':
    case '*':
    case '^':
    case '~':
    case '!':
    case '=':
    case ':':
    case '%':
    case '-':
    case '+':
    case '&':
    case '|':
    case '/':
    case '<':
    case '>':
    case '.':
	return true;
    }
    return false;
}


#define advance(state) { trans = &&state; goto advancePointer; }


size_t cppTokenizer::tokenize( const char *line, size_t n )
{
    size_t first = 0;
    size_t last = first;
    bool multiline = false;
    void *trans = state;
    const char *p = line;
    if( std::distance(line,p) >= n ) return n;
    if( trans != NULL ) goto *trans; else goto token;
    
advancePointer:
    ++p;
    switch( (std::distance(line,p) >= n) ? '\0' : *p ) {
    case '\\': 
		while( *p == '\r' ) ++p; 
		if( *p == '\n' ) goto exit;
		break; 
    case '\r': 
		while( *p == '\r' ) ++p; 
		assert( *p == '\n' | *p == '\0' );
    case '\n':  
    case '\0':  
		/* In a multi line comment, end-of-line characters 
		   are not classified as separators. */
		if( !multiline ) trans = NULL; 
		goto exit;		
    } 
    goto *trans;

charEnd:
    switch( *p ) {
    case '\'':
	advance(token);
    }
    goto error;

charEscSeqTail:
    switch( *p ) {
    case '\'':
    case '\"':
    case '\?':
    case '\\':
    case '\a':
    case '\b':
    case '\f':
    case '\n':
    case '\r':
    case '\t':
    case '\v':
	advance(charEnd);
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
	advance(charOctEscSeqTail);
    case 'x':
	advance(charHexEscSeqTail);
    case 'u':
	advance(charUnivCharNameTail);
    }
    goto error;

charOctEscSeqEnd:
    switch( *p ) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
	advance(charEnd);
    case '\'':
	goto charEnd;	
    }
    goto error;

charOctEscSeqTail:
    switch( *p ) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
	advance(charOctEscSeqEnd);
    case '\'':
	goto charEnd;	
    }    
    goto error;

charHexEscSeqTail:
    switch( *p ) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
	goto charHexEscSeqTail; 
    }
    goto charEnd;

charHexQuad:
    switch( *p ) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
	++hexQuads;
	advance(charHexQuad);
    }
    if( hexQuads != 4 | hexQuads != 8 ) goto error;
    goto charEnd;

charTail:
    switch( *p ) {
    case '\\':
	/* simple-escape-sequence, octal-escape-sequence, hexadecimal-escape-sequence */
	advance(charEscSeqTail);
    }
    advance(charEnd);

charUnivCharNameTail:
    switch( *p ) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
	hexQuads = 1;
	advance(charHexQuad);
    }
    goto error;

colonTail:
	switch( *p ) {
	case '>':
	case ':':
	    advance(token);
	}
	goto token;

decFloTail:
    switch( *p ) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
	/* decimal-literal, floating-literal */
	advance(decFloTail);
    case '.':
	advance(fractionalConstantTail);
    case 'e':
    case 'E': 
	advance(expTail);
    }
    goto token;

digitTail:
    switch( *p ) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
	advance(digitTail);
    }
    goto error;

divideTail:
    switch( *p ) {
    case '=':
	advance(token);
    case '/':
    tok = cppComment;
	advance(lineCommentTail);
    case '*':
    tok = cppComment;
	advance(multiLineCommentTail);
    }
    goto token;

duplicate:
    if( *p == expects ) {
		advance(token);
    }
    goto token;

duplicateOrEqual:
    if( *p == expects | *p == '=' ) {
	advance(token);
    }
    goto token;

ellipsisTail:
    if( *p == '.' ) {
	advance(token);
    }
    goto error;

error:
    /* skip until next space */
    if( isSeparator(*p) ) goto token; 
    advance(error);

exit:
    last = std::distance(line,p);
    if( last - first > 0  && listener != NULL ) {
	listener->token(tok,line,first,last,trans != NULL);
	first = last;
    }
    if( *p == '\n' ) {
	++p;
	last = std::distance(line,p);
	listener->newline(line,first,last);
	first = last;
    }
    state = trans;
    if( last >= n ) return last;
    if( trans != NULL ) goto *trans;
    goto token;    

expTail:
    switch( *p ) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
	advance(digitTail);
    case '+':
    case '-':
	advance(expDigSequence);
    }
    goto error;

expDigSequence:
    switch( *p ) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
	advance(digitTail);
    }
    goto error;

floHexOctTail:
    switch( *p ) {
    case 'x':
    case 'X':
	/* hexadecimal-literal */
	advance(hexTail);
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
	/* floating-literal, octal-literal */
	advance(floOctTail);
    case '8':
    case '9':
	/* floating-literal */
	advance(floTail);
    }
	tok = cppDecimalLiteral;
    goto token;

floOctTail:
    switch( *p ) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
	/* floating-literal, octal-literal */
	advance(floOctTail);
    case '8':
    case '9':
	/* floating-literal */
	advance(floTail);   
    case 'e':
    case 'E':
	advance(expTail);   
    case '.':	
	advance(fractionalConstantTail);
    }
    goto token;

floPrepOp:
    switch( *p ) {
    case '.':
	tok = cppOperator;
	advance(ellipsisTail);
    case '*':
	tok = cppOperator;
	advance(token);
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
	tok = cppFloatingLiteral;
	advance(fractionalConstantTail);
    }
    goto token;

floTail:
    switch( *p ) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
	/* floating-literal */
	advance(floTail);   
    case 'e':
    case 'E':
	advance(expTail);   
    case '.':	
	advance(fractionalConstantTail);
    }
    goto token;
 
fractionalConstantTail:
    switch( *p ) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
	advance(fractionalConstantTail);
    case 'e':
    case 'E':
	advance(expTail);
    case 'f':
    case 'l':
    case 'F':
    case 'L':
	advance(token);
    }
    goto token;
       
greaterThanEnd:
    switch( *p ) {
    case '=':
	advance(token);
    }
    goto token;

hexTail:
    switch( *p ) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
	advance(hexTail);
    }
    if( isSeparator(*p) ) goto token;
    goto error;

identifierTail:
    if( isSeparator(*p) ) {
		goto token;
	}
    advance(identifierTail);

lessThanEnd:
    switch( *p ) {
    case '=':
	advance(token);
    }
    goto token;

lessThanTail:
    switch( *p ) {
    case ':':
    case '%':
    case '=':
    case '<':
	advance(lessThanEnd);
    }
    goto token;
    
lineCommentTail:
    switch( *p ) {
    case '\n':
	goto token;
    }
    advance(lineCommentTail);


minusTail:
    switch( *p ) {
    case '-':
    case '=':
	advance(token);
    case '>':
	advance(pointerTail);
    }
    goto token;
    
multiLineCommentEnd:
    switch( *p ) {
    case '/':
		multiline = false;
		advance(token);
    }
    advance(multiLineCommentTail);

multiLineCommentTail:
    if( *p =='*' ) {
		advance(multiLineCommentEnd);
    }
	multiline = true;
	advance(multiLineCommentTail);
    
pointerTail:
    if( *p == '*' ) advance(token);
    goto token;
    
remainTail:
    switch( *p ) {
    case '>':
    case ':':
    case '=':
	advance(token);
    }
    goto token;

stringEscSeqTail:
    switch( *p ) {
    case '\'':
    case '\"':
    case '\?':
    case '\\':
    case '\a':
    case '\b':
    case '\f':
    case '\n':
    case '\r':
    case '\t':
    case '\v':
	advance(stringTail);
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
	advance(stringOctEscSeqTail);
    case 'x':
	advance(stringHexEscSeqTail);
    case 'u':
	advance(stringUnivCharNameTail);
    }
    goto error;
    
stringHexEscSeqTail:
    switch( *p ) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
	goto stringHexEscSeqTail; 
    }
    goto stringTail;

stringHexQuad:
    switch( *p ) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
	++hexQuads;
	advance(stringHexQuad);
    }
    if( hexQuads != 4 | hexQuads != 8 ) goto error;
    goto stringTail;

stringOctEscSeqEnd:
    switch( *p ) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
		advance(stringTail);
    case '"':
		goto stringTail;	
    }
    goto error;
	
 stringOctEscSeqTail:
    switch( *p ) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
		advance(stringOctEscSeqEnd);
    case '"':
		goto stringTail;	
    }
    goto error;

stringTail:
    switch( *p ) {
    case '"':
		advance(token);
    case '\\':
		advance(stringEscSeqTail);
    }
    advance(stringTail);
	
stringUnivCharNameTail:
    switch( *p ) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
	hexQuads = 1;
	advance(stringHexQuad);
    }
    goto error;

spaceTail:
    if( *p == ' ' ) advance(spaceTail);
    goto token;

tabSpaceTail:
    if( *p == '\t' ) advance(tabSpaceTail);
    goto token;
 
token:
    if( trans != NULL ) {
	if( tok == cppIdentifier ) {
	    tok = identifierToken(std::string(&line[first],p - &line[first]));
	}
	if( listener != NULL ) {
	    listener->token(tok,line,first,p - line,false);
	}
    }
    tok = cppErr;
    first = p - line;
	switch( *p ) {
    case 'L':
		/* character-literal, identifier, string-literal */
		advance(wideCharTail);
    case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
    case 'G':
    case 'H':
    case 'I':
    case 'J':
    case 'K':
    case 'M':
    case 'N':
    case 'O':
    case 'P':
    case 'Q':
    case 'R':
    case 'S':
    case 'T':
    case 'U':
    case 'V':
    case 'W':
	case 'X':
	case 'Y':
	case 'Z':
    case '_':
#if 0
    case univsersal_character_name:
    case other_imp_def_chars:
#endif
		/* identifier, keyword, preprocessing-op-or-punc, boolean-literal */
		tok = cppIdentifier;
		advance(identifierTail);
    case '0':
		/* pp-number, octal-literal, hexadecimal-literal, floating-literal */
		advance(floHexOctTail);
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
	/* decimal-literal, floating-literal */
	advance(decFloTail);
    case '\'':
		/* character-literal */
		advance(charTail);
    case '"':
		/* header-name, string-literal */
		tok = cppStringLiteral;
		advance(stringTail);
	// header-name: isSourceChar(*p) & !( *p == '\n' | *p == '"' ) +
    case '{':
    case '}':
    case '[':
    case ']':
    case '(':
    case ')':
    case ';':
    case ',':
	/* preprocessing-op-or-punc */
	tok = cppPunctuator;
	advance(token);
    case '#':
	tok = cppPreprocessing;
	expects = '#';
	advance(duplicate);
    case '*':
    case '^':
    case '~':
    case '!':
    case '=':
	tok = cppOperator;
	expects = '=';
	advance(duplicate);
    case ':':
	tok = cppPunctuator;
	advance(colonTail);
    case '%':
	tok = cppOperator;
	advance(remainTail);
    case '-':
	tok = cppOperator;
	advance(minusTail);
    case '+':
    case '&':
    case '|':
	tok = cppOperator;
	expects = *p;
	advance(duplicateOrEqual);
    case '/':
	/* comment, preprocessing-op-or-punc */
	advance(divideTail);
    case '>':
	tok = cppOperator;
	advance(greaterThanEnd);
    case '<':
	/* header-name, preprocessing-op-or-punc */
	tok = cppPreprocessing;
	advance(lessThanTail);
    case '.':
	/* floating-literal, preprocessing-op-or-punc */
	tok = cppOperator;
	advance(floPrepOp)   
    case ' ':
	tok = cppSpace;
	advance(spaceTail);
    case '\t':
	tok = cppTabSpace;
	advance(tabSpaceTail);
	case '\n':
	case '\0':
	    /* empty lines */
	    goto exit;
    }
    goto error;

wideCharTail:
    switch( *p ) {
    case '\'':
	/* character-literal */
	tok = cppCharacterLiteral;
	advance(charTail);
    case '"':
	/* string-literal */
	tok = cppStringLiteral;
	advance(stringTail);
    default:
	tok = cppIdentifier;
	advance(identifierTail);
    }
    goto error;
}


