/* Copyright (c) 2011-2013, Fortylines LLC
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

/** tokenizer for the Internet Message Format
    (http://tools.ietf.org/html/rfc2822)

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/

namespace tero {

const char *rfc2822TokenTitles[] = {
    "err",
    "fieldName",
    "colon",
    "fieldBody",
    "messageBody",
    "messageBreak"
};

#define advance(state) { trans = &&state; goto advancePointer; }


size_t rfc2822Tokenizer::tokenize( const char *line, size_t n )
{
    size_t first = 0;
    size_t last;
    void *trans = state;
    const char *p = line, *crlf = NULL;
    if( n == 0 ) return n;
    if( trans != NULL ) goto *trans; else goto entryPoint;

advancePointer:
    last = std::distance(line, p);
    if( *p == '\0' ) {
        if( listener != NULL ) {
            listener->token(tok, line, first, p - line, false);
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
    switch( *p ) {
    case '\r':
        tok = rfc2822MessageBody;
        advance(headersCR);
        break;
    case '\n':
        tok = rfc2822MessageBody;
        advance(bodyCR);
        break;
    default:
        break;
    }
    tok = rfc2822FieldName;
    advance(fieldName);

headersCR:
    if( *p == '\n' ) {
        advance(bodyCR);
    }
    advance(headers);

entryPoint:
    if( *p == 'F' ) {
        advance(bodyFromF);
    }
    goto headers;

body:
    /* We are now in the body of the message. */
    switch( *p ) {
    case '\r':
        advance(bodyCR);
        break;
    case '\n':
        advance(bodyCR);
        break;
    }
    advance(body);

bodyCR:
    if( *p == 'F' ) {
        advance(bodyFromF);
    }
    goto body;

bodyFromF:
    if( *p == 'r' ) advance(bodyFromR);
    goto body;

bodyFromR:
    if( *p == 'o' ) advance(bodyFromO);
    goto body;

bodyFromO:
    if( *p == 'm' ) advance(bodyFromM);
    goto body;

bodyFromM:
    if( (p - line - 4) > 0 && listener != NULL ) {
        listener->token(tok, line, first, p - line - 4, false);
    }
    first = p - line - 4;
    tok = rfc2822MessageBreak;
    goto messageBreak;

messageBreak:
    while( *p != '\r' && *p != '\n' ) {
        advance(messageBreak);
    }
    advance(messageBreakCR);

messageBreakCR:
    if( listener != NULL ) {
        listener->token(tok, line, first, p - line - 1, false);
    }
    first = p - line;
    last = std::distance(line, p);
    state = &&headers;
    return last;
    //    goto headers;
}

}

