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

#ifndef guardrfc5545
#define guardrfc5545

#include <iostream>
#include <iterator>

/**
   Display a calendar as an HTML page.
   See: iCalendar - RFC5545 (http://tools.ietf.org/html/rfc5545)

   Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/

enum rfc5545Token {
    rfc5545Err,
    rfc5545FieldName,
    rfc5545Colon,
    rfc5545FieldBody
};


extern const char *rfc5545TokenTitles[];

template<typename ch, typename tr>
inline std::basic_ostream<ch, tr>&
operator<<( std::basic_ostream<ch, tr>& ostr, rfc5545Token v ) {
    return ostr << rfc5545TokenTitles[v];
}


/** Interface for callbacks from the rfc5545Tokenizer
 */
class rfc5545TokListener {
public:
    rfc5545TokListener() {}

    virtual void newline(const char *line, int first, int last ) = 0;

    virtual void token( rfc5545Token token, const char *line,
        int first, int last, bool fragment ) = 0;
};


template<typename charT, typename traitsT = std::char_traits<charT> >
class htmlrfc5545TokListener : public rfc5545TokListener {
protected:
    std::basic_ostream<charT,traitsT> *ostr;

public:
    explicit htmlrfc5545TokListener( std::basic_ostream<charT,traitsT>& o )
        : ostr(&o) {}

public:
    void newline(const char *line, int first, int last ) {
        *ostr << std::endl;
    }

    void token( rfc5545Token token, const char *line,
        int first, int last, bool fragment ) {
        *ostr << "<span class=\"" << rfc5545TokenTitles[token] << "\">";
        std::copy(&line[first],&line[last],std::ostream_iterator<charT>(*ostr));
        *ostr << "</span>";
    }
};


/** Tokenizer for rfc5545 (calendars)
 */
class rfc5545Tokenizer {
protected:
    void *state;
    rfc5545Token tok;
    rfc5545TokListener *listener;

public:
    rfc5545Tokenizer()
        : state(NULL), tok(rfc5545Err), listener(NULL) {}

    explicit rfc5545Tokenizer( rfc5545TokListener& l )
        : state(NULL), tok(rfc5545Err), listener(&l) {}

    void attach( rfc5545TokListener& l ) { listener = &l; }

    size_t tokenize( const char *line, size_t n );
};

#endif
