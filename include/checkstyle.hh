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

#ifndef guardcheckstyle
#define guardcheckstyle

#include "slice.hh"
#include "cpptok.hh"
#include "shtok.hh"
#include "projfiles.hh"

/**
   Check the style conventions for source files in a project.

   Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


enum licenseCode {
    unknownLicense,
    BSDLicense
};

extern const char *licenseCodeTitles[];

template<typename ch, typename tr>
inline std::basic_ostream<ch, tr>&
operator<<( std::basic_ostream<ch, tr>& ostr, licenseCode v ) {
    return ostr << licenseCodeTitles[v];
}


class checker {
public:
    slice<const char> licenseText;

protected:
    enum stateCode {
	start,
	readLicense,
	doneLicense
    };
    stateCode state;

    bool cached;
    licenseCode licenseType;

    void cache();

public:
    /* \todo get through accessors */
    size_t nbLines;
    size_t nbCodeLines;
    
public:
    checker() : state(start), cached(false), 
		licenseType(unknownLicense),
		nbLines(0), nbCodeLines(0) {}

    licenseCode license() {
	if( !cached ) cache();
	return licenseType;
    }
};


class cppChecker : public checker,
		     public cppTokListener {
protected:    
    enum commentCode {
	emptyLine,
	codeLine,
	commentLine
    };
    commentCode comment;

    cppTokenizer tokenizer;

public:
    cppChecker();

    virtual void newline(const char *line, int first, int last );

    virtual void token( cppToken token, const char *line, 
			int first, int last, bool fragment );

    size_t tokenize( const char *line, size_t n ) {
	return tokenizer.tokenize(line,n);
    }
};


class shChecker : public checker,
		    public shTokListener {
protected:
    shTokenizer tokenizer;


public:
    shChecker() : tokenizer(*this) {}

    virtual void newline(const char *line, int first, int last );

    virtual void token( shToken token, const char *line, 
			int first, int last, bool fragment );

    size_t tokenize( const char *line, size_t n ) {
	return tokenizer.tokenize(line,n);
    }
};


template<typename checker>
class checkfile : public document {
public:
    virtual void fetch( session& s, const boost::filesystem::path& pathname ) const;

    virtual void meta( session& s, const boost::filesystem::path& pathname ) const {}
};


typedef checkfile<cppChecker> cppCheckfile;
typedef checkfile<shChecker> shCheckfile;


class checkstyle : public projfiles {
protected:    
    virtual void 
    addDir( session& s, const boost::filesystem::path& pathname ) const;

    virtual void 
    addFile( session& s, const boost::filesystem::path& pathname ) const;

    virtual void flush( session& s ) const;
    
public:
    template<typename iter>
    checkstyle( iter first, iter last ) 
	: projfiles(first,last) {}
};


#include "checkstyle.tcc"


#endif
