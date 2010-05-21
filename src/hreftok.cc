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

/* <a href="http://www.w3.org/TR/html5/">HTML5 Reference</a> */

#include <cassert>
#include <iostream>
#include "hreftok.hh"

const char *hrefTokenTitles[] = {
    "hrefErr",
    "hrefFilename",
    "hrefSpace",
    "hrefText"
};


#define advance(state) { trans = &&state; goto advancePointer; }


size_t hrefTokenizer::tokenize( const char *line, size_t n )
{
    void *trans = state;
    const char *p = line;
    bool newline = false;
    int last;
    first = 0;

    if( n == 0 ) return 0;
    if( trans != NULL ) goto *trans; else goto token;
    
advancePointer:
    ++p;
    last = std::distance(line,p);
    switch( ((size_t)last >= n) ? '\0' : *p ) {
    case '\r': 
	while( *p == '\r' ) ++p; 
	assert( (*p == '\n') | (*p == '\0') );
    case '\n':  
	++p;
	newline = true;
    case '\0':
	if( *p == '\0' ) ++p;
	if( last - first > 0 ) {
	    if( listener != NULL ) listener->token(tok,line,first,last,true);
	    first = last;
	}
	last = std::distance(line,p);
	if( newline ) {
	    if(  listener != NULL ) listener->newline(line,first,last);
	    first = last;	    
	    trans = &&token;
	}
	state = trans;	
    } 
    if( (size_t)last >= n ) return last;
    goto *trans;

 filename:
    switch( *p ) {
    case ' ':
    case '\t':
    case '(':
    case ')':
	goto token;
    case '.':
	tok = hrefText;
	advance(maybeFilenameDot);
    case '/':
	tok = hrefText;
	advance(maybeFilenameDirSep);
    }    
    advance(filename);

 maybeFilenameDot:
    switch( *p ) {
    case ' ':
    case '\t':
    case '(':
    case ')':
	goto token;
    case '.':
	advance(maybeFilenameDot2);
    case '/':
	advance(maybeFilenameDirSep);	
    }
    tok = hrefFilename;
    advance(filename);

 maybeFilenameDot2:
    switch( *p ) {
    case '/':
	advance(maybeFilenameDirSep);
    }
    advance(token);

 maybeFilenameDirSep:
    switch( *p ) {
    case ' ':
    case '\t':
    case '(':
    case ')':
	goto token;
    case '.':
	advance(maybeFilenameDot);
    case '/':
	advance(text);	
    }
    tok = hrefFilename;
    advance(filename);

 spaces:
    if( (*p == ' ') | (*p == '\t') ) advance(spaces);
    goto token;

 text:
    switch( *p ) {
    case ' ':
    case '\t':
	goto token;
    case '.':
	advance(maybeFilenameDot);
    case '/':
	advance(maybeFilenameDirSep);
    }
    advance(text);
        
 token:
    last = std::distance(line,p);
    if( last > first && listener != NULL ) {
	listener->token(tok,line,first,last,false);
	trans = NULL;
    }
    tok = hrefErr;
    first = last;
    if( (*p == '\n') | (*p == '\r') | (*p == '\0') ) {
	--p;
	advance(token);
    }
    switch( *p ) {
    case '\t':
    case ' ':
	tok = hrefSpace;
	advance(spaces);
    default:
	tok = hrefText;
	advance(text);
    }
}

