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

#ifndef guardcheckstyle
#define guardcheckstyle

#include "slice.hh"
#include "cpptok.hh"
#include "shtok.hh"
#include "document.hh"

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


class checkfile : public document {
public:
    slice<const char> licenseText;

protected:
    bool cached;
    licenseCode licenseType;

    void cache();

public:
    /* \todo get through accessors */
    size_t nbLines;
    size_t nbCodeLines;
    
public:
    checkfile();

    licenseCode license() {
	if( !cached ) cache();
	return licenseType;
    }

    virtual void fetch( session& s, const boost::filesystem::path& pathname );

    virtual void meta( session& s, const boost::filesystem::path& pathname ) {}
};


class cppCheckfile : public checkfile,
		     public cppTokListener {
protected:
    enum stateCode {
	start,
	readLicense,
	doneLicense
    };
    stateCode state;
    
    enum commentCode {
	emptyLine,
	codeLine,
	commentLine
    };
    commentCode comment;

public:
    cppCheckfile();

    virtual void newline(const char *line, int first, int last );

    virtual void token( cppToken token, const char *line, 
			int first, int last, bool fragment );

    virtual void fetch( session& s, const boost::filesystem::path& pathname );
};


class shCheckfile : public checkfile,
		    public shTokListener {
protected:
    enum stateCode {
	start,
	readLicense,
	doneLicense
    };
    stateCode state;

public:
    shCheckfile() {}

    virtual void newline(const char *line, int first, int last );

    virtual void token( shToken token, const char *line, 
			int first, int last, bool fragment );

    virtual void fetch( session& s, const boost::filesystem::path& pathname );
};

#endif
