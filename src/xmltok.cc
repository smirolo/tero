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

#include <cassert>
#include <iostream>
#include "xmltok.hh"


const char *xmlTokenTitles[] = {
    "xmlErr",
    "xmlComment",
    "xmlDeclEnd",
    "xmlDeclStart",
    "xmlName",
    "xmlSpace",
    "xmlAssign",
    "xmlContent",
    "xmlElementEnd",
    "xmlElementStart",
    "xmlAttValue"
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
	tok = xmlElementEnd;
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
	advance(endEmptyElement);
    case '>':
	tok = xmlElementEnd;
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
