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

#ifndef guardaws
#define guardaws

#include <stdint.h>
#include <boost/uuid/uuid.hpp>
#include "webserve.hh"
#include "markup.hh"
#include "session.hh"
#include <boost/uuid/uuid_io.hpp>

class awsStandardButton {
public:
    typedef std::map<std::string,std::string> paramMap;

protected:
    std::string accessKey;
    std::string secretKey;
    std::string certificate;
    uint32_t amount;
    boost::uuids::uuid referenceId;
    std::string signature;
    std::string signatureMethod;
    uint32_t signatureVersion;

    static const char *paypipeline;
    static const char *image;

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
    awsStandardButton( const std::string& accessKey,
		       const std::string& secretKey,
		       const std::string& certificate );

    void build( boost::uuids::uuid referenceId, uint32_t amount );

    /** returns true when the signature of the return request 
	matches the one computed with the server's secretKey. */
    bool checkReturn( const session& s,
		      const char *requestURI );
    
    template<typename ch, typename tr>
    void writehtml( std::basic_ostream<ch, tr>& ostr ) const;
};


template<typename ch, typename tr>
void awsStandardButton::writehtml( std::basic_ostream<ch, tr>& ostr ) const {
    std::stringstream refid;
    refid << referenceId;

    ostr << html::form().action(paypipeline).method("POST");
    if( !abandonUrl.empty() )
	ostr << html::input()
	    .type(html::input::hidden)
	    .nameref("abandonUrl")
	    .value(abandonUrl.string());

    ostr << html::input()
	.type(html::input::hidden)
	.nameref("accessKey")
	.value(accessKey)

	/* \todo "USD 5" */
	 << html::input()
	.type(html::input::hidden)
	.nameref("amount")
	.value(amount)

	 << html::input()
	.type(html::input::hidden)
	.nameref("cobrandingStyle")
	.value(cobrandingStyle)

	 << html::input()
	.type(html::input::hidden)
	.nameref("collectShippingAddress")
	.value(collectShippingAddress)

	 << html::input()
	.type(html::input::hidden)
	.nameref("description")
	.value(description)

	 << html::input()
	.type(html::input::hidden)
	.nameref("immediateReturn")
	.value(immediateReturn);

    if( !ipnUrl.empty() )
	ostr << html::input()
	    .type(html::input::hidden)
	    .nameref("ipnUrl")
	    .value(ipnUrl.string());

    ostr << html::input()
	.type(html::input::hidden)
	.nameref("processImmediate")
	.value(processImmediate)
	 << html::input()
	.type(html::input::hidden)
	.nameref("referenceId")
	.value(refid.str());

    if( !returnUrl.empty() )
	ostr << html::input()
	    .type(html::input::hidden)
	    .nameref("returnUrl")
	    .value(returnUrl.string());

    /* write the signature generated for the button */
    ostr << html::input()
	.type(html::input::hidden)
	.nameref("signature")
	.value(signature)
	 << html::input()
	.type(html::input::hidden)
	.nameref("signatureMethod")
	.value(signatureMethod)
	 << html::input()
	.type(html::input::hidden)
	.nameref("signatureVersion")
	.value(signatureVersion);

    ostr << html::input()
	.type(html::input::image)
	.src(image)

	 << html::form::end;
}



#endif
