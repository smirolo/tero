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

#ifndef guardmarkdown
#define guardmarkdown

#include <iterator>

/**
   tokenizer for Markdown (http://daringfireball.net/projects/markdown)

   Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


enum mdToken {
    mdErr,
    mdHeader,
    mdListItem,
    mdPara,
};

extern const char *mdTokenTitles[];

template<typename ch, typename tr>
inline std::basic_ostream<ch, tr>&
operator<<( std::basic_ostream<ch, tr>& ostr, mdToken v ) {
    return ostr << mdTokenTitles[v];
}


/** Interface for callbacks from the mdTokenizer
 */
class mdTokListener {
public:
    mdTokListener() {}

    virtual void newline(const char *line, int first, int last ) = 0;

    virtual void token( mdToken token, const char *line,
        int first, int last, bool fragment ) = 0;
};


template<typename charT, typename traitsT = std::char_traits<charT> >
class htmlmdTokListener : public mdTokListener {
protected:
    std::basic_ostream<charT,traitsT> *ostr;

public:
    explicit htmlmdTokListener( std::basic_ostream<charT,traitsT>& o )
        : ostr(&o) {}

public:
    void newline(const char *line, int first, int last ) {
        *ostr << std::endl;
    }

    void token( mdToken token, const char *line,
        int first, int last, bool fragment ) {
        *ostr << "<span class=\"" << mdTokenTitles[token] << "\">";
        std::copy(&line[first],&line[last],
            std::ostream_iterator<charT>(*ostr));
        *ostr << "</span>";
    }
};


/** Tokenizer for md
 */
class mdTokenizer {
protected:
    void *state;
    mdToken tok;
    mdTokListener *listener;

public:
    mdTokenizer()
        : state(NULL), tok(mdErr), listener(NULL) {}

    explicit mdTokenizer( mdTokListener& l )
        : state(NULL), tok(mdErr), listener(&l) {}

    void attach( mdTokListener& l ) { listener = &l; }

    size_t tokenize( const char *line, size_t n );
};

#endif
