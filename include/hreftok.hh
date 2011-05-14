/* Copyright (c) 2009-2011, Fortylines LLC
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

#ifndef guardhreftok
#define guardhreftok

#include <iterator>

/** The href tokenizer attempts to recognize filenames in a text. 
    The decorators can then thus generate hypertext links out of them.

    \todo extend to recognize urls.

   Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/
enum hrefToken {
    hrefErr,
    hrefFilename,
    hrefSpace,
    hrefText
};

extern const char *hrefTokenTitles[];

template<typename ch, typename tr>
inline std::basic_ostream<ch, tr>&
operator<<( std::basic_ostream<ch, tr>& ostr, hrefToken v ) {
    return ostr << hrefTokenTitles[v];
}


/** Interface for callbacks from the hrefTokenizer
 */
class hrefTokListener {
public:
    hrefTokListener() {}
    
    virtual void newline( const char *line, 
			  int first, int last ) = 0;
    
    virtual void token( hrefToken token, const char *line, 
			int first, int last, bool fragment ) = 0;
};


class hrefTokenizer {
protected:
	void *state;
	int first;
	hrefToken tok;
	hrefTokListener *listener;

public:
    hrefTokenizer() 
	: state(NULL), first(0), tok(hrefErr), listener(NULL) {}

    hrefTokenizer( hrefTokListener& l ) 
	: state(NULL), first(0), tok(hrefErr), listener(&l) {}
    
    void attach( hrefTokListener& l ) { listener = &l; }

    size_t tokenize( const char *line, size_t n );
};


#endif
