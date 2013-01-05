/* Copyright (c) 2010-2013, Fortylines LLC
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

#include "paypal.hh"
#include <cryptopp/oids.h>
#include <cryptopp/hmac.h>
#include <cryptopp/rsa.h>
#include <cryptopp/sha.h>
#include <cryptopp/base64.h>
#include <cryptopp/files.h>
#include <cryptopp/hex.h>

#include <cryptopp/smartptr.h>
#include <cryptopp/modes.h>
#include <cryptopp/aes.h>
#if 0
#include <cryptopp/rijndael.h>
#endif
#include <cryptopp/cryptlib.h>

/** Paypal Payment Services.

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


namespace {

    void Load(const std::string& filename, CryptoPP::BufferedTransformation& bt)
{
        // http://www.cryptopp.com/docs/ref/class_file_source.html
        CryptoPP::FileSource file(filename.c_str(), true /*pumpAll*/);

        file.TransferTo(bt);
        bt.MessageEnd();
}

    void LoadPrivateKey(const std::string& filename, CryptoPP::PrivateKey& key)
{
        // http://www.cryptopp.com/docs/ref/class_byte_queue.html
        CryptoPP::ByteQueue queue;

	std::cerr << "!!! load private key from " << filename << std::endl;

        Load(filename, queue);
        key.Load(queue);        
}



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
	using namespace boost::system::errc;
	boost::throw_exception(boost::system::system_error(
	    make_error_code(no_such_file_or_directory),pathname));
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
const char *paypalStandardButton::paypipeline 
    = "https://www.paypal.com/cgi-bin/webscr";
#else
const char *paypalStandardButton::paypipeline 
    = "https://www.paypal.com/cgi-bin/webscr";
#endif

const char *paypalStandardButton::image 
= "https://www.paypal.com/en_US/i/btn/btn_buynow_LG.gif";

static const char *httpMethod = "GET";
static const char *hostHeader = "https://authorize.payments-sandbox.amazon.com";
static const char *requestURI = "/pba/paypipeline";

sessionVariable paypalSecretKey("paypalSecretKey","Paypal Private Key File");
sessionVariable paypalPublicCertificate("paypalPublicCertificate","Certificate for the paypal API public key");

void 
paypalAddSessionVars( boost::program_options::options_description& opts,
			boost::program_options::options_description& visible ) {
    using namespace boost::program_options;
    options_description localOpts("paypal");
    localOpts.add(paypalSecretKey.option());
    localOpts.add(paypalPublicCertificate.option());
    opts.add(localOpts);
    visible.add(localOpts);
}


paypalStandardButton::paypalStandardButton( const session& s )
    : secretKeyPath(paypalSecretKey.value(s)),
      certificate(paypalPublicCertificate.value(s)),
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
paypalStandardButton::loadSecretKey( const boost::filesystem::path& p ) {
    using namespace boost::filesystem;
    using namespace CryptoPP;

    std::stringstream sg;
    ifstream pem(p);
    if( pem.fail() ) {
	using namespace boost::filesystem;
	using namespace boost::system::errc;
	boost::throw_exception(boost::system::system_error(
	    make_error_code(no_such_file_or_directory),p.string()));
    }
    std::string line;
    std::getline(pem,line);
    while( !pem.eof() ) {
	if( line.compare(0,5,"-----") != 0 ) {
	    sg << line;
	}
	std::getline(pem,line);
    }

    std::string result;
    CryptoPP::StringSource((const byte*)sg.str().c_str(),
			   sg.str().size(),
			   true,
			  new CryptoPP::Base64Decoder(new StringSink(result)));
    return result;   
}



std::string 
paypalStandardButton::formatRequest( const std::string& httpMethod,
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


void paypalStandardButton::build( const std::string& r, uint32_t a ) {

    using namespace CryptoPP;

    std::stringstream s;

    referenceId = r;
    amount = a;

    /*
      certPath = args[0];
      keyPath = args[1];
      paypalCertPath = args[2];
      keyPass = args[3];
      cmdText = args[4];
      output = args[5];

      ClientSide client_side 
      = new ClientSide( keyPath, certPath, paypalCertPath, keyPass );
      
      String result = client_side.getButtonEncryptionValue( cmdText, 
      keyPath, certPath, paypalCertPath, keyPass );

      getButtonEncryptionValue():
      # fortylines-paypal-pubcert.pem
      X509Certificate certificate 
          = cf.generateCertificate( new FileInputStream(_certPath) );
	  
      signedGenerator.addSigner( privateKey, certificate, 
                      CMSSignedDataGenerator.DIGEST_SHA1 );
      CMSSignedData signedData = signedGenerator.generate(cmsByteArray, true, "BC");

      RFC 3852

    */
    
    /* create the request to send to Paypal. */
    std::stringstream request;
    request << "cert_id=XF4YCV7SZUJ2Q\n";
    request << "cmd=_xclick\nbusiness=info@fortylines.com\nitem_name=Handheld Computer\nitem_number=1234\ncustom=sc-id-789\namount=500.00\ncurrency_code=USD\ntax=41.25\nshipping=20.00\naddress_override=1\naddress1=123 Main St\ncity=Austin\nstate=TX\nzip=94085\ncountry=US\nno_note=1cancel_return=http://www.fortylines.com/cancel.htm\n";

    /* Sign the request with our private key. 

       signedGenerator.addSigner( privateKey, certificate, 
       CMSSignedDataGenerator.DIGEST_SHA1 );

       See CMS (PKCS7/RFC 3852) - http://tools.ietf.org/html/rfc3852 */

    RSA::PrivateKey secretKey;
    LoadPrivateKey(secretKeyPath.string(),secretKey);

    ByteQueue apiPublicKeyBytes;
    GetPublicKeyFromCert(certificate.c_str(),apiPublicKeyBytes);

#if 1
    /* PKCS7 *PKCS7_encrypt(STACK_OF(X509) *certs, BIO *in, const
       EVP_CIPHER *cipher, int flags); 
       PKCS #1 v1.5 padded encryption
       scheme based on RSA. 
       PKCS7 http://tools.ietf.org/html/rfc5652 */
    /* RSASignFile(const char *privFilename, 
       const char *messageFilename, const char *signatureFilename)
    */
    {
	using namespace CryptoPP;

	/* RandomNumberGenerator & GlobalRNG() */
	static OFB_Mode<AES>::Encryption s_globalRNG;
#if 0
	std::string seed = IntToString(time(NULL));
#else
	std::string seed = "defaultseed";
#endif
	seed.resize(16);
	s_globalRNG.SetKeyWithIV((byte *)seed.data(), 16, (byte *)seed.data());

#if 0
    RSASS<PKCS1v15, SHA>::Signer priv(secretKey);    
    StringSource((const byte*)request.str().data(),request.str().size(), true,
  new SignerFilter(s_globalRNG, priv, new HexEncoder(new FileSink(std::cerr))));
#endif
    /*
      CMSEnvelopedDataGenerator envGenerator = new CMSEnvelopedDataGenerator();
      envGenerator.addKeyTransRecipient(payPalCert);
      CMSEnvelopedData envData 
          = envGenerator.generate(new CMSProcessableByteArray(signed),
	                 CMSEnvelopedDataGenerator.DES_EDE3_CBC, "BC" );
      byte[] pkcs7Bytes = envData.getEncoded();
      return new String( DERtoPEM(pkcs7Bytes, "PKCS7") );
     */

#if 0
    /* http://www.mail-archive.com/cryptopp-users@googlegroups.com/msg00095.html */
    /* from ~/build/cryptopp-5.6.0/validat1.cpp */
    const byte iv[] = {0x12,0x34,0x56,0x78,0x90,0xab,0xcd,0xef};

    DES_EDE3_Encryption en(apiPublicKeyBytes);
    CBC_CTS_Encryptor encryptor(en, iv);
    
    memcpy(&keyStruct->key_iv[0], apiPublicKeyBytes, DES_EDE3_KEYSIZE);
    memcpy(&keyStruct->key_iv[(int)DES_EDE3_KEYSIZE], iv, 
	   DES_EDE3_Encryption::BLOCKSIZE);
    FileSink fs1(payloadFileName);
    fs1.Put((const byte *) &keyResult->key_iv [0], sizeof(keyResult->key_iv));
#endif


    }

#endif
}

    
bool paypalStandardButton::checkReturn( const session& s,
				     const char *requestURI ) 
{    
}
