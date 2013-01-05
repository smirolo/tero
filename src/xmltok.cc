/* Copyright (c) 2009-2013, Fortylines LLC
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

#include <cassert>
#include "tokenize.hh"

/** XML tokenizer

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


const char *xmlTokenTitles[] = {
    "xmlErr",
    "xmlAssign",
    "xmlAttValue",
    "xmlCloseTag",
    "xmlComment",
    "xmlContent",
    "xmlDeclEnd",
    "xmlDeclStart",
    "xmlElementEnd",
    "xmlElementStart",
    "xmlEmptyElementEnd",
    "xmlName",
    "xmlSpace"
};


#define advance(state) { trans = &&state; goto advancePointer; }

bool nameStartChar( int c ) {
    return isalpha(c);
}

bool nameChar( int c ) {
    return isalnum(c) | (c == ':');
}

size_t xmlTokenizer::tokenize( const char *line, size_t n )
{
    const char *p = line;
    int last;
    first = 0;

    if( n == 0 ) return 0;
    if( trans != NULL ) goto *trans; else goto token;

advancePointer:
	last = (size_t)std::distance(line,p);
	if( last >= n ) {
		if( last > first && listener != NULL ) {
			listener->token(tok,line,first,last,true);
		}
		return last;
	}
    switch( *p ) {
    case '\\':
		savedtrans = trans;
		trans = &&esceol;
		goto eolAdvancePointer;
    case '\r':
		/* windows and internet protocols */
		savedtrans = trans;
		trans = &&eolmacwin;
		goto eolAdvancePointer;
    case '\n':
		/* unix-like */
		savedtrans = trans;
		trans = &&eol;
		goto eolAdvancePointer;
    case '\0':
		goto eolAdvancePointer;
    }
	++p;
    goto *trans;

 eolAdvancePointer:
	last = (size_t)std::distance(line,p);
	if( last >= n || *p == '\0' ) {
		if( last > first && listener != NULL ) {
			listener->token(tok,line,first,last,true);
		}
		return last;
	}
	++p;
    goto *trans;

 esceol:
	/* Escaped EOL character */
	switch( *p ) {
	case '\r':
		/* windows and internet protocols */
		trans = &&eolmacwin;
		goto eolAdvancePointer;
	case '\n':
		/* unix-like */
		trans = &&eol;
		goto eolAdvancePointer;
	}
	/* not an escaped newline */
	trans = savedtrans;
	savedtrans = NULL;
	goto *trans;

 eolmacwin:
	switch( *p ) {
	case '\n':
		trans = &&eol;
		goto eolAdvancePointer;
	}
	goto eol;

 eol:
	if( last > first && listener != NULL ) {
		listener->token(tok,line,first,last,false);
	}
	first = last;
	last = (size_t)std::distance(line,p);
	if( last > first && listener != NULL ) {
		listener->newline(line,first,last);
	}
	first = last;
	trans = savedtrans;
	savedtrans = NULL;
	goto token;

	/* specialized tokenizer */

attValueDouble:
    /* AttValue ::= '"' ([^<&"] | Reference)* '"' */
    if( *p != '"' ) advance(attValueDouble);
    advance(tag);

attValueSingle:
    /* AttValue ::= "'" ([^<&'] | Reference)* "'" */
    if( *p != '"' ) advance(attValueSingle);
    advance(tag);

comment:
    if( *p == '-') advance(endComment);
    advance(comment);

content:
    if( *p != '<' ) advance(content);
    goto token;

endComment:
    if( *p == '-') advance(endComment2);
    advance(comment);

endComment2:
    switch( *p ) {
    case '>': advance(token);
    case '-': advance(endComment2);
    }
    advance(comment);

 endEmptyDecl:
    if( *p == '>' ) {
	tok = xmlDeclEnd;
	advance(token);
    }
    goto error;

 endEmptyElement:
    if( *p == '>' ) {
	tok = xmlEmptyElementEnd;
	advance(token);
    }
    goto error;

 error:
    /* skip until tag delimiter character */
    if( (*p != '<') & (*p != '>') ) advance(error);
    goto tag;

 name:
    if( nameChar(*p) ) advance(name);
    goto tag;

 spaces:
    if( *p == ' ' ) advance(spaces);
    goto tag;

 spacesBeforeContent:
    if( *p == ' ' ) advance(spacesBeforeContent);
    goto token;

startComment:
    if( *p == '-') advance(startComment2);
    goto error;

startComment2:
    if( *p == '-') advance(comment);
    goto error;

 startDecl:
    switch( *p ) {
    case '/':
	tok = xmlElementEnd;
	advance(tag);
    case '?':
	tok = xmlDeclStart;
	advance(tag);
    case '!':
	tok = xmlComment;
	advance(startComment);
    }
    tok = xmlElementStart;
    goto tag;

 tag:
    if( (p - line) > first && listener != NULL ) {
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
    case '\t':
    case ' ':
	tok = xmlSpace;
	advance(spaces);
    case '<':
	advance(startDecl);
    case '/':
	advance(endEmptyElement);
    case '>':
	tok = xmlCloseTag;
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
    case '?':
	advance(endEmptyDecl);
    }
    goto error;

 token:
    last = std::distance(line,p);
    if( last > first && listener != NULL ) {
		listener->token(tok,line,first,last,false);
		trans = NULL;
    }
    tok = xmlErr;
    first = last;
    switch( *p ) {
#if 0
	/* \todo It was introduced for spaces after tag at end of line
	   but this does not suit very well with formatted text. */
    case '\t':
    case ' ':
	tok = xmlSpace;
	advance(spacesBeforeContent);
#endif
    case '<':
		goto tag;
    }
    tok = xmlContent;
    advance(content);
}
