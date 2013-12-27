/* Copyright (c) 2013, Fortylines LLC
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

#include "markdown.hh"

/** Markdown tokenizer

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/

namespace tero {

const char *mdTokenTitles[] = {
    "err",
    "header",
    "listItem",
    "para",
};

#define advance(state) { trans = &&state; goto advancePointer; }


size_t mdTokenizer::tokenize( const char *line, size_t n )
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

headerOrListItem:
    if( *p == '-' ) {
        advance(header)
    }
    goto listItem;

headerOrEqual:
    if( *p == '=' ) {
        advance(header)
    }
    goto para;

header:
    tok = mdHeader;
    switch( *p ) {
    case '-':
    case '=':
    case '#':
        advance(header)
    }
    goto entryPoint;

listItem:
    tok = mdListItem;
    if( *p == '\r' || *p == '\n' ) goto entryPoint;
    advance(listItem);

lineEndCR:
    if( *p == '\n' ) advance(lineEnd);
    goto entryPoint;

lineEnd:
    first = crlf - line;
    if( listener != NULL ) {
        listener->newline(line, first, p - line);
    }
    first = p - line;
    goto entryPoint;

para:
    if( *p == '\r' || *p == '\n' ) goto entryPoint;
    advance(para);

entryPoint:
    last = std::distance(line,p);
    if( trans != NULL ) {
        if( last - first > 0 && listener != NULL ) {
            listener->token(tok, line, first, last, false);
        }
    }
    trans = NULL;
    first = last;
    tok = mdPara;
    switch( *p ) {
    case '-':
        advance(headerOrListItem);
    case '=':
        advance(headerOrEqual);
    case '#':
        advance(header);
    case '\r':
        crlf = p;
        advance(lineEndCR);
    case '\n':
        crlf = p;
        advance(lineEnd);
    }
    advance(para);
}

}

