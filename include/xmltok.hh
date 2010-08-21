/* Copyright (c) 2009, Fortylines LLC
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

#ifndef guardxmltok
#define guardxmltok

#include <iterator>

/** The current XML tokenizer recognize elements, data, comments 
    and declaration nodes.

    As XML Elements are complex entities beyhond the scope of a lexical
    tokenizer, the tokens generated are: xmlElementStart ('<'), 
    xmlName (alphanum identifier), xmlAssign, xmlAttValue ("..."), 
    xmlCloseTag ('>'), xmlElementEnd ('</') and xmlEmptyElementEnd ('/>').

    xmlError is the first element such that xmlErr == 0, thus consistent 
    with memset-style initialization.
    
    \todo Add rapidxml::node_cdata, rapidxml::node_doctype 
          (and rapidxml::node_pi?)

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/
enum xmlToken {
    xmlErr,
    xmlAssign,
    xmlAttValue,
    xmlCloseTag,
    xmlComment,
    xmlContent,
    xmlDeclEnd,
    xmlDeclStart,
    xmlElementEnd,
    xmlElementStart,
    xmlEmptyElementEnd,
    xmlName,
    xmlSpace
};

extern const char *xmlTokenTitles[];

template<typename ch, typename tr>
inline std::basic_ostream<ch, tr>&
operator<<( std::basic_ostream<ch, tr>& ostr, xmlToken v ) {
    return ostr << xmlTokenTitles[v];
}


/** Interface for callbacks from the xmlTokenizer
 */
class xmlTokListener {
public:
    xmlTokListener() {}
    
    virtual void newline( const char *line, 
			  int first, int last ) = 0;
    
    virtual void token( xmlToken token, const char *line, 
			int first, int last, bool fragment ) = 0;
};


class xmlTokenizer {
protected:
	void *state;
	int first;
	xmlToken tok;
	int hexQuads;
	char expects;
	xmlTokListener *listener;

public:
    xmlTokenizer() 
	: state(NULL), first(0), tok(xmlErr), listener(NULL) {}

    xmlTokenizer( xmlTokListener& l ) 
	: state(NULL), first(0), tok(xmlErr), listener(&l) {}
    
    void attach( xmlTokListener& l ) { listener = &l; }

    size_t tokenize( const char *line, size_t n );
};


#endif
