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
#include "shtok.hh"

#define advance(state) { trans = &&state; goto advancePointer; }

size_t shTokenizer::tokenize( const char *line, size_t n )
{
    size_t first = 0;
    void *trans = state;
    const char *p = line;
    if( std::distance(line,p) >= n ) return n;
    if( trans != NULL ) goto *trans; else goto token;
    
advancePointer:
    ++p;
    switch( (std::distance(line,p) >= n) ? '\0' : *p ) {
    case '\r': 
	while( *p == '\r' ) ++p; 
	assert( *p == '\n' | *p == '\0' );
    case '\n':  
    case '\0':  
	trans = &&token;		
    } 
    if( std::distance(line,p) >= n ) {
	state = trans;
	return n;
    }
    goto *trans;


code:
    advance(code);

comment:   
    advance(comment);

token:
    if( trans != NULL ) {
	if( listener != NULL ) {
	    listener->token(tok,line,first,p - line,false);
	}
    }
    tok = shErr;
    first = p - line;
    switch( *p ) {
    case '#':
	advance(comment);
	break;
    default:
	advance(code);
    }
}
