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

#include "aws.hh"
#include <cryptopp/oids.h>
#include <cryptopp/hmac.h>
#include <cryptopp/rsa.h>
#include <cryptopp/sha.h>
#include <cryptopp/base64.h>
#include <cryptopp/files.h>
#include <cryptopp/hex.h>

/** Amazon Payment Services.

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


namespace {

/** Reads an X.509 v3 certificate from a PEM file, extracts 
    the subjectPublicKeyInfo structure (which is one way PK_Verifiers 
    can get their key material) and writes it to keyout.

    @throws CryptoPP::BERDecodeError

    This code is derived from what was posted at 
    http://www.cryptopp.com/wiki/X.509 and originally offered 
    by Geoff Beier on the Crypto++ newsgroup.
*/

void GetPublicKeyFromCert( const char* pathname,
			   CryptoPP::BufferedTransformation& keyout )
{
    using namespace CryptoPP;

    std::stringstream sg;
    std::ifstream pem(pathname);
    if( pem.fail() ) {
	using namespace boost::filesystem;
	boost::throw_exception(
	        basic_filesystem_error<path>(std::string("file not found"),
							    pathname, 
					     boost::system::error_code()));
    }
    std::string line;
    std::getline(pem,line);
    while( !pem.eof() ) {
	if( line.compare(0,5,"-----") != 0 ) {
	    sg << line;
	}
	std::getline(pem,line);
    }
    CryptoPP::StringSource certin((const byte*)sg.str().c_str(),
				  sg.str().size(),
				  true,
				  new CryptoPP::Base64Decoder());       
    
    BERSequenceDecoder x509Cert(certin);
    BERSequenceDecoder tbsCert(x509Cert);
    
    // ASN.1 from RFC 3280
    // TBSCertificate  ::=  SEQUENCE  {
    // version         [0]  EXPLICIT Version DEFAULT v1,
    
    // consume the context tag on the version
    BERGeneralDecoder context(tbsCert,0xa0);
    word32 ver;
    
    // only want a v3 cert
    BERDecodeUnsigned<word32>(context,ver,INTEGER,2,2);
    
    // serialNumber         CertificateSerialNumber,
    Integer serial;
    serial.BERDecode(tbsCert);

    // signature            AlgorithmIdentifier,
    BERSequenceDecoder signature(tbsCert);
    signature.SkipAll();
    
    // issuer               Name,
    BERSequenceDecoder issuerName(tbsCert);
    issuerName.SkipAll();
    
    // validity             Validity,
    BERSequenceDecoder validity(tbsCert);
    validity.SkipAll();
    
    // subject              Name,
    BERSequenceDecoder subjectName(tbsCert);
    subjectName.SkipAll();
    
    // subjectPublicKeyInfo SubjectPublicKeyInfo,
    BERSequenceDecoder spki(tbsCert);
    DERSequenceEncoder spkiEncoder(keyout);
    
    spki.CopyTo(spkiEncoder);
    spkiEncoder.MessageEnd();
    
    spki.SkipAll();
    tbsCert.SkipAll();
    x509Cert.SkipAll();
}

}


#if 1
const char *awsStandardButton::paypipeline 
    = "https://authorize.payments-sandbox.amazon.com/pba/paypipeline";
#else
const char *awsStandardButton::paypipeline 
    = "https://authorize.payments.amazon.com/pba/paypipeline";
#endif

const char *awsStandardButton::image 
    = "http://g-ecx.images-amazon.com/images/G/01/asp/"
	"golden_small_paynow_withmsg_whitebg.gif";

const char *httpMethod = "GET";
const char *hostHeader = "https://authorize.payments-sandbox.amazon.com";
const char *requestURI = "/pba/paypipeline";

awsStandardButton::awsStandardButton( const std::string& ak,
				      const std::string& sk,
				      const std::string& cf )
    : accessKey(ak),
      secretKey(sk),
      certificate(cf),
      amount(0),
      referenceId(),
      signature(),
      signatureMethod("HmacSHA256"),
      signatureVersion(2),
      abandonUrl(""),
      cobrandingStyle("logo"),
      collectShippingAddress(false),
      description(""),
      immediateReturn(false),
      ipnUrl(""),
      isDonationWidget(false),
      processImmediate(true),
      returnUrl("")
{
}

std::string 
awsStandardButton::formatRequest( const std::string& httpMethod,
				  const std::string& hostHeader,
				  const std::string& requestURI,
				  const paramMap& params,
				  const char *sep )
{
    const char *sepUsed = "";
    std::stringstream s;
    /*
      StringToSign = HTTPVerb + "\n" +
      ValueOfHostHeaderInLowercase + "\n" +
      HTTPRequestURI + "\n" +         
      CanonicalizedQueryString  

      * @param httpMethod - POST or GET
      * @param hostHeader - Service end point
      * @param requestURI - Path
    */    
    s << httpMethod << '\n'
      << hostHeader << '\n'
      << requestURI << '\n';

    for( paramMap::const_iterator param = params.begin(); 
	 param != params.end();++param ) {
	s << sepUsed << param->first << "=" << param->second;
	sepUsed = sep;
    } 
    return s.str();
}


void awsStandardButton::build( const std::string& r, uint32_t a ) {
    std::stringstream s;

    referenceId = r;
    amount = a;

    paramMap params;
    if( !abandonUrl.empty() ) params["abandonUrl"] = abandonUrl.string();
    params["accessKey"] = accessKey;
    s << amount;
    params["amount"] = s.str();
    params["cobrandingStyle"] = cobrandingStyle;
    params["collectShippingAddress"] = collectShippingAddress;
    params["description"] = description;
    params["immediateReturn"] = immediateReturn;
    if( !ipnUrl.empty() ) params["ipnUrl"] = ipnUrl.string();
    params["processImmediate"] = processImmediate;
    s.str("");
    s << referenceId;
    params["referenceId"] = s.str();
    if( !returnUrl.empty() ) params["returnUrl"] = returnUrl.string();

    std::string request 
	= formatRequest(httpMethod,hostHeader,requestURI,params);
    
    /* Calculate an RFC 2104-compliant HMAC with the previously created string,
       the Secret Access Key, and SHA256 or SHA1 as the hash algorithm.
       (reference: http://www.ietf.org/rfc/rfc2104.txt)

       Crypto++ Library 5.6.0 - http://www.cryptopp.com/ */
    typedef CryptoPP::HMAC<CryptoPP::SHA256> HMAC_SHA256;
    HMAC_SHA256 md((const byte*)secretKey.c_str(),secretKey.size());
    CryptoPP::SecByteBlock digest(md.DigestSize());
    md.Update((const byte*)request.c_str(),request.size());
    md.Final(digest);

    /*
    CryptoPP::Base64Encoder b64e;
    b64e.Update(digest);
    
    From http://www.cryptopp.com/wiki/User_Guide:_base64.h
    */
    CryptoPP::StringSource((const byte*)digest,digest.size(), true,
	   new CryptoPP::Base64Encoder(
	       new CryptoPP::StringSink(signature),false));
}

    
bool awsStandardButton::checkReturn( const session& s,
				     const char *requestURI ) 
{    
    using namespace CryptoPP;

    /* Read the public key out of the PEM certificate. */
    ByteQueue keyBytes;
    try {
	GetPublicKeyFromCert(certificate.c_str(),keyBytes);
    } catch( std::exception& ) {
	std::cerr << 
	    "Failed to extract the public key from the CA certificate." 
		  << std::endl;
	return false;
    }

    /* Recreate the message that was signed. */
    std::string signatureS;
    paramMap params;
    const char *httpMethod = "GET";				     
    const char *hostHeader = s.valueOf("domainName").c_str();

    for( session::variables::const_iterator p = s.query.begin();
	 p != s.query.end(); ++p ) {
	if( p->first == std::string("signature") ) {
	    signatureS = std::string(p->second);
	} else {
	    params[p->first] = uriEncode(p->second);
	}
    }
    std::string request 
	= formatRequest(httpMethod,hostHeader,requestURI,params,"&");
#if 0
    std::cerr << "- request to sign:" << std::endl;
    std::cerr << request << std::endl;
    std::cerr << "- expected signature: " << std::endl;
    std::cerr << signatureS << std::endl;
#endif
    /* Verify the signature attached to the request. */
    CryptoPP::StringSource signatureFile((const byte*)signatureS.c_str(),
					 signatureS.size(), true, 
					 new CryptoPP::Base64Decoder());

    CryptoPP::RSASS<CryptoPP::PKCS1v15, CryptoPP::SHA>::Verifier pub(keyBytes);
    if( signatureFile.MaxRetrievable() != pub.SignatureLength() ) {
	std::cerr << "error: expected signature of " << pub.SignatureLength()
		  << " bytes but only " << signatureFile.MaxRetrievable() 
		  << " available." << std::endl; 
	return false;
    }

    CryptoPP::SecByteBlock signature(pub.SignatureLength());
    signatureFile.Get(signature, signature.size());
    
    CryptoPP::VerifierFilter *verifierFilter = new CryptoPP::VerifierFilter(pub);
    verifierFilter->Put(signature, pub.SignatureLength());
    CryptoPP::StringSource msg((const byte*)request.c_str(),
			       request.size(),true,verifierFilter);

    return verifierFilter->GetLastResult();
}
