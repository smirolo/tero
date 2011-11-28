/* Copyright (c) 2011, Fortylines LLC
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

#include <cstdlib>
#include <cassert>
#include <string>
#include <algorithm>
#include <iostream>
#include <boost/regex.hpp>
#include "rfc2822tok.hh"

/** tokenizer for the Internet Message Format (http://tools.ietf.org/html/rfc2822)

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


const char *rfc2822TokenTitles[] = {
    "err",
    "fieldName",
    "colon",
    "fieldBody",
    "messageBody"
};

#define advance(state) { trans = &&state; goto advancePointer; }


size_t rfc2822Tokenizer::tokenize( const char *line, size_t n )
{
    size_t first = 0;
    size_t last;
    void *trans = state;
    const char *p = line, *crlf = NULL;
    if( n == 0 ) return n;
    if( trans != NULL ) goto *trans; else goto headers;
    
advancePointer:
    last = std::distance(line,p);
    if( *p == '\0' ) {
		if( listener != NULL ) {
			listener->token(tok,line,first,p - line,false);
		}
    }
    if( last >= n ) {
		state = trans;
		return last;
    }
    ++p;
    goto *trans;

fieldName:
    while( *p != ':' ) advance(fieldName);
    if( listener != NULL ) {
	listener->token(tok,line,first,p - line,false);
    }
    tok = rfc2822Colon;
    first = p - line;
    advance(headerColon);

headerColon:
    if( listener != NULL ) {
	listener->token(tok,line,first,p - line,false);
    }
    tok = rfc2822FieldBody;
    first = p - line;
    advance(fieldBody);

fieldBody:
    /* The standard says it should always be a CR/LF pair but for convenience,
       we will also accept LF by itself. */
    if( *p == '\r' ) { crlf = p; advance(fieldBodyCR); }
    if( *p == '\n' ) { crlf = p; advance(fieldBodyCRLF); }
    advance(fieldBody);

fieldBodyCR:
    if( *p == '\n' ) advance(fieldBodyCRLF);
    advance(fieldBody);

fieldBodyCRLF:
    if( *p == ' ' || *p == '\t' ) advance(fieldBody);
    if( listener != NULL ) {
	listener->token(tok,line,first,crlf - line,false);
    }
    tok = rfc2822Err;
    first = crlf - line;
    if( listener != NULL ) {
	listener->newline(line,first,p - line);
    }
    first = p - line;
    goto headers;    

headers:
    /* The standard says it should always be a CR/LF pair but for convenience,
       we will also accept LF by itself. */
    if( *p == '\r' ) {
	tok = rfc2822MessageBody;
	advance(headersCR);
    }
    if( *p == '\n' ) {
	tok = rfc2822MessageBody;
	advance(headersCRLF);
    }
    tok = rfc2822FieldName;
    advance(fieldName);

headersCR:
    if( *p == '\n' ) {
	advance(headersCRLF);
    }
    advance(headers);    
   
headersCRLF:
    /* We are now in the body of the message. */
    advance(headersCRLF);

}

