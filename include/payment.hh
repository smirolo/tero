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

#ifndef guardpayment
#define guardpayment

#include "document.hh"
#include "adapter.hh"

class payment : public document {
protected:
    struct entry {
	const boost::regex* regexp;
	const char *retPath;
	adapter* adapt; 

	entry() {}

	entry( const boost::regex& r, const char *rp, adapter& a ) 
	    : regexp(&r), retPath(rp), adapt(&a) {}

    };

    typedef std::vector<entry> entrySeq;

    entrySeq entries;

public:
    explicit payment( std::ostream& o ) 
	: document(o) {}

    void add( const boost::regex& r, const char *retPath, adapter& a );

    static void 
    addSessionVars( boost::program_options::options_description& opts );

    static void checkReturn( session& s, const char* page );

    void fetch( session& s, const boost::filesystem::path& pathname );    

    static void show( std::ostream& ostr,
		      session& s, 
		      const url& returnUrl, 
		      const std::string& referenceId, 
		      size_t value,
		      const std::string& descr = "" );
};


#endif
