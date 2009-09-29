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

#ifndef guardcpptok
#define guardcpptok

#include <iterator>

enum cppToken {
    cppErr,
    cppBooleanLiteral,
    cppCharacterLiteral,
    cppFloatingLiteral,
    cppDecimalLiteral,
    cppOctalLiteral,
    cppHexadecimalLiteral,
    cppIdentifier,
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


/** Interface for callbacks from the cppTokenizer
 */
class cppTokListener {
public:
	cppTokListener() {}

	virtual void newline() = 0;

	virtual void token( cppToken token, const char *line, 
						int first, int last, bool fragment ) = 0;
};


class xmlCppTokListener : public cppTokListener {
protected:
	std::ostream *ostr;
	
public:
	explicit xmlCppTokListener( std::ostream& o ) : ostr(&o) {}
	
	void token( cppToken token, const char *line, 
				int first, int last, bool fragment ) {
		*ostr << '<' << cppTokenTitles[token];
		if( fragment ) *ostr << " fragment=\"" << fragment << "\"";
		*ostr << " text=\"[" << first << "," << last << "]\">";
		std::copy(&line[first],&line[last],std::ostream_iterator<char>(*ostr));
		*ostr << "</" << cppTokenTitles[token] << ">";
	}
};


template<typename charT, typename traitsT>
class htmlCppTokListener : public cppTokListener {
protected:
	std::basic_ostream<charT,traitsT> *ostr;
	
public:
	explicit htmlCppTokListener( std::basic_ostream<charT,traitsT>& o ) 
		: ostr(&o) {}
	
public:
	void token( cppToken token, const char *line, 
				int first, int last, bool fragment ) {
		*ostr << "<span class=\"" << cppTokenTitles[token] << "\">";
		std::copy(&line[first],&line[last],
				  std::ostream_iterator<charT>(*ostr));
		*ostr << "</span>";
	}

	void endl() {
		*ostr << std::endl;
	}
};


/** Tokenizer for C/C++ source files
 */
class cppTokenizer {
protected:
	void *state;
	int first;
	cppToken tok;
	int hexQuads;
	char expects;
	cppTokListener *listener;

public:
    cppTokenizer() 
		: state(NULL), first(0), tok(cppErr), listener(NULL) {}
	
    explicit cppTokenizer( cppTokListener& l ) 
	: state(NULL), first(0), tok(cppErr), listener(&l) {}
    
    void attach( cppTokListener& l ) { listener = &l; }
    
    void tokenize( const char *line, size_t n );
};

#endif
