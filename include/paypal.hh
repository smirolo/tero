/* Copyright (c) 2010-2011, Fortylines LLC
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

#ifndef guardpaypal
#define guardpaypal

#include <stdint.h>
#include "webserve.hh"
#include "markup.hh"
#include "session.hh"

/**
   Generate a button to checkout though Amazon Payment System.

   Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
 */

extern sessionVariable paypalSecretKey;
extern sessionVariable paypalPublicCertificate;

void 
paypalAddSessionVars( boost::program_options::options_description& opts,
		      boost::program_options::options_description& visible );


class paypalStandardButton {
public:
    typedef std::map<std::string,std::string> paramMap;

protected:
    boost::filesystem::path secretKeyPath;
    std::string certificate;
    uint32_t amount;
    std::string referenceId;
    std::string signature;
    std::string signatureMethod;
    uint32_t signatureVersion;

    static const char *paypipeline;
    static const char *image;

    std::string loadSecretKey( const boost::filesystem::path& p );

public:   
    static
    std::string formatRequest( const std::string& httpMethod,
			       const std::string& hostHeader,
			       const std::string& requestURI,
			       const paramMap& params,
			       const char *sep = "" );

public:
    url abandonUrl;
    std::string cobrandingStyle;
    bool collectShippingAddress;
    std::string description;
    bool immediateReturn;
    url ipnUrl;
    bool isDonationWidget;
    bool processImmediate;
    url returnUrl;
    
public:
    explicit paypalStandardButton( const session& s );

    void build( const std::string& referenceId, uint32_t amount );

    /** returns true when the signature of the return request 
	matches the one computed with the server's secretKey. */
    bool checkReturn( const session& s,
		      const char *requestURI );
    
    template<typename ch, typename tr>
    void writehtml( std::basic_ostream<ch, tr>& ostr ) const;
};


template<typename ch, typename tr>
void paypalStandardButton::writehtml( std::basic_ostream<ch, tr>& ostr ) const {
    std::stringstream refid;
    refid << referenceId;

    ostr << html::form().action(url(paypipeline)).method("post");

    ostr << html::input()
	.type(html::input::hidden)
	.nameref("business")
	.value("info@fortylines.com")
	
	 << html::input()
	.type(html::input::hidden)
	.nameref("cmd")
	.value("_xclick")

	 << html::input()
	.type(html::input::image)
	.nameref("submit")
	.src(url(image));

    /* Specify details about the item that buyers will purchase. */
    ostr << html::input()
	.type(html::input::hidden)
	.nameref("item_name")
	.value(refid.str())

	.type(html::input::hidden)
	.nameref("amount")
	.value(amount)

	.type(html::input::hidden)
	.nameref("currency_code")
	.value("USD");
	
    ostr << html::form::end;
}

#endif
