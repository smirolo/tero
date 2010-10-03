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

#ifndef guardpayment
#define guardpayment

#include "document.hh"
#include "adapter.hh"

/**
   Interaction with payment system 

   Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


class payCheckoutButton {
public:
    typedef std::map<std::string,std::string> paramMap;

    std::string description;
    url returnUrl;

protected:
    uint32_t amount;
    std::string referenceId;

    const url image;    
    const url paypipeline;

public:
    explicit payCheckoutButton( url domainName );
    virtual ~payCheckoutButton() {}

   void build( const std::string& referenceId, uint32_t amount );

    /** returns true when the signature of the return request 
	matches the one computed with the server's secretKey. */
    bool checkReturn( const session& s, const char *requestURI );
    
    template<typename ch, typename tr>
    void writehtml( std::basic_ostream<ch, tr>& ostr ) const;
};



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


/** "Do-nothing" pipeline to process payments 
 */
class payPipeline : public document {
protected:
public:
    explicit payPipeline( std::ostream& o ) 
	: document(o) {}

    void fetch( session& s, const boost::filesystem::path& pathname );    

};


#endif
