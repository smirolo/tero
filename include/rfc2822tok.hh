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

#ifndef guardrf2822tok
#define guardrf2822tok

#include <iterator>

/**
   tokenizer for the Internet Message Format (http://tools.ietf.org/html/rfc2822)

   Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


enum rfc2822Token {
    rfc2822Err,
    rfc2822FieldName,
    rfc2822Colon,
    rfc2822FieldBody,
    rfc2822MessageBody,
    rfc2822MessageBreak
};

extern const char *rfc2822TokenTitles[];

template<typename ch, typename tr>
inline std::basic_ostream<ch, tr>&
operator<<( std::basic_ostream<ch, tr>& ostr, rfc2822Token v ) {
    return ostr << rfc2822TokenTitles[v];
}


/** Interface for callbacks from the cppTokenizer
 */
class rfc2822TokListener {
public:
    rfc2822TokListener() {}

    virtual void newline(const char *line, int first, int last ) = 0;

    virtual void token( rfc2822Token token, const char *line,
        int first, int last, bool fragment ) = 0;
};


template<typename charT, typename traitsT = std::char_traits<charT> >
class htmlrfc2822TokListener : public rfc2822TokListener {
protected:
    std::basic_ostream<charT,traitsT> *ostr;

public:
    explicit htmlrfc2822TokListener( std::basic_ostream<charT,traitsT>& o )
        : ostr(&o) {}

public:
    void newline(const char *line, int first, int last ) {
        *ostr << std::endl;
    }

    void token( rfc2822Token token, const char *line,
        int first, int last, bool fragment ) {
        *ostr << "<span class=\"" << rfc2822TokenTitles[token] << "\">";
        std::copy(&line[first],&line[last],
            std::ostream_iterator<charT>(*ostr));
        *ostr << "</span>";
    }
};


/** Tokenizer for rfc2822
 */
class rfc2822Tokenizer {
protected:
    void *state;
    rfc2822Token tok;
    rfc2822TokListener *listener;

public:
    rfc2822Tokenizer()
        : state(NULL), tok(rfc2822Err), listener(NULL) {}

    explicit rfc2822Tokenizer( rfc2822TokListener& l )
        : state(NULL), tok(rfc2822Err), listener(&l) {}

    void attach( rfc2822TokListener& l ) { listener = &l; }

    size_t tokenize( const char *line, size_t n );
};

#endif
