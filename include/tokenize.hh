/* Copyright (c) 2009-2012, Fortylines LLC
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

#ifndef guardtokenize
#define guardtokenize

#include <iterator>

/**
   Tokenizers for the different text files in our source repository.

   Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


enum cppToken {
    cppErr,
    cppBooleanLiteral,
    cppCharacterLiteral,
    cppFloatingLiteral,
    cppDecimalLiteral,
    cppOctalLiteral,
    cppHexadecimalLiteral,
    cppIdentifier,
    cppIncorrectIdentifier,
    cppKeyword,
    cppOperator,
    cppPunctuator,
    cppComment,
    cppSpace,
    cppTabSpace,
    cppPreprocessing,
    cppStringLiteral
};

extern const char *cppTokenTitles[];

template<typename ch, typename tr>
inline std::basic_ostream<ch, tr>&
operator<<( std::basic_ostream<ch, tr>& ostr, cppToken v ) {
    return ostr << cppTokenTitles[v];
}


enum errToken {
	errErr,
	errFilename,
	errLineNum,
	errMessage,
	errSeparator
};


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


enum xmlEscToken {
    escErr,
    escAmpEscape,
    escData,
    escGtEscape,
    escLtEscape,
    escQuotEscape
};

extern const char *xmlEscTokenTitles[];

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


/** Interface for callbacks from the cppTokenizer
 */
class cppTokListener {
public:
    cppTokListener() {}
    
    virtual void newline(const char *line, 
			  int first, int last ) = 0;
    
    virtual void token( cppToken token, const char *line, 
			int first, int last, bool fragment ) = 0;
};


/** Tokenizer for C/C++ source files
 */
class cppTokenizer {
protected:
	void *state;
	cppToken tok;
	int hexQuads;
	char expects;
	cppTokListener *listener;

public:
    cppTokenizer() 
		: state(NULL), tok(cppErr), listener(NULL) {}
	
    explicit cppTokenizer( cppTokListener& l ) 
	: state(NULL), tok(cppErr), listener(&l) {}
    
    void attach( cppTokListener& l ) { listener = &l; }
    
    size_t tokenize( const char *line, size_t n );
};


/** Interface for callbacks from the errTokenizer
 */
class errTokListener {
public:
    errTokListener() {}
    
    virtual void newline(const char *line, 
			  int first, int last ) = 0;
    
    virtual void token( errToken token, const char *line, 
			int first, int last, bool fragment ) = 0;
};


/** Tokenizer for compiler error/warning messages

	Implementation Note: We use this tokenizer for parsable pylint output.
 */
class errTokenizer {
protected:
	void *trans, *savedtrans;
	errToken tok;
	errTokListener *listener;

public:
    errTokenizer() 
		: trans(NULL), savedtrans(NULL), tok(errErr), listener(NULL) {}
	
    explicit errTokenizer( errTokListener& l ) 
		: trans(NULL), savedtrans(NULL), tok(errErr), listener(&l) {}
    
    void attach( errTokListener& l ) { listener = &l; }
    
    size_t tokenize( const char *line, size_t n );
};


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


/** The href tokenizer attempts to recognize filenames in a text. 
    The decorators can then thus generate hypertext links out of them.

    \todo extend to recognize urls.
*/
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


/** Shell like text tokenizer.

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/

enum shToken {
    shErr,
    shComment,
    shCode
};

extern const char *shTokenTitles[];

template<typename ch, typename tr>
inline std::basic_ostream<ch, tr>&
operator<<( std::basic_ostream<ch, tr>& ostr, shToken v ) {
    return ostr << shTokenTitles[v];
}


/** Interface for callbacks from the shTokenizer
 */
class shTokListener {
public:
    shTokListener() {}
    
    virtual void newline(const char *line, 
			  int first, int last ) = 0;
    
    virtual void token( shToken token, const char *line, 
			int first, int last, bool fragment ) = 0;
};


/** Tokenizer for shell-based (sh, python, Makefile) source files
 */
class shTokenizer {
protected:
	void *state;
	shToken tok;
	shTokListener *listener;

public:
    shTokenizer() 
	: state(NULL), tok(shErr), listener(NULL) {}
	
    explicit shTokenizer( shTokListener& l ) 
	: state(NULL), tok(shErr), listener(&l) {}
    
    void attach( shTokListener& l ) { listener = &l; }
    
    size_t tokenize( const char *line, size_t n );
};


/** Interface for callbacks from the xmlTokenizer
 */
class xmlEscTokListener {
public:
    xmlEscTokListener() {}
    
    virtual void newline( const char *line, int first, int last ) = 0;
    
    virtual void token( xmlEscToken token, const char *line, 
			int first, int last, bool fragment ) = 0;
};


/** XML escaper tokenizer.
*/
class xmlEscTokenizer {
protected:
	void *trans, *savedtrans;
	xmlEscToken tok;
	xmlEscTokListener *listener;

public:
    xmlEscTokenizer() 
	: trans(NULL), tok(escErr), listener(NULL) {}

    xmlEscTokenizer( xmlEscTokListener& l ) 
	: trans(NULL), tok(escErr), listener(&l) {}
    
    void attach( xmlEscTokListener& l ) { listener = &l; }

    size_t tokenize( const char *line, size_t n );
};


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

*/
class xmlTokenizer {
protected:
	void *trans, *savedtrans;
	int first;
	xmlToken tok;
	int hexQuads;
	char expects;
	xmlTokListener *listener;

public:
    xmlTokenizer() 
	: trans(NULL), first(0), tok(xmlErr), listener(NULL) {}

    xmlTokenizer( xmlTokListener& l ) 
	: trans(NULL), first(0), tok(xmlErr), listener(&l) {}
    
    void attach( xmlTokListener& l ) { listener = &l; }

    size_t tokenize( const char *line, size_t n );
};

#endif
