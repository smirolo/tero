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

#include "payment.hh"
#include "todo.hh"
#include "aws.hh"

/** Payment processing pipeline

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/

payCheckoutButton::payCheckoutButton( url domainName )
    : image("/resources/tryit.png"), 
      paypipeline(domainName.string() + "/paypipeline")
{}


void payCheckoutButton::build( const std::string& r, uint32_t a )
{
    referenceId = r;
    amount = a;
}

 
bool payCheckoutButton::checkReturn( const session& s, const char *requestURI )
{
    return true;
}
    
 
template<typename ch, typename tr>
void payCheckoutButton::writehtml( std::basic_ostream<ch, tr>& ostr ) const 
{
    std::stringstream refid;
    refid << referenceId;

    ostr << html::form().action(paypipeline).method("POST");

    ostr << html::input()
	.type(html::input::hidden)
	.nameref("amount")
	.value(amount)

	 << html::input()
	.type(html::input::hidden)
	.nameref("description")
	.value(description)

	 << html::input()
	.type(html::input::hidden)
	.nameref("referenceId")
	.value(refid.str());

    if( !returnUrl.empty() )
	ostr << html::input()
	    .type(html::input::hidden)
	    .nameref("returnUrl")
	    .value(returnUrl.string());

    ostr << html::input()
	.type(html::input::image)
	.src(image)

	 << html::form::end;
}



void payment::add( const boost::regex& r, 
		   const char *retPath, adapter& a ) {
    entries.push_back(entry(r,retPath,a));
}


void 
payment::addSessionVars( boost::program_options::options_description& opts ) {
    using namespace boost::program_options;
    /* For authentication with Amazon payment services. */
    options_description amazonOpts("amazon");
    amazonOpts.add_options()
	("awsAccessKey",value<std::string>(),"Amazon Access Key")
	("awsSecretKey",value<std::string>(),"Amazon Secret Key")
	("awsCertificate",value<std::string>(),"Amazon Public Certificate")
	("paypalSecretKey",value<std::string>(),"Paypal Private Key File")
	("paypalPublicCertificate",value<std::string>(),
	 "Certificate for the paypal API public key")
	("referenceId",value<std::string>(),
	 "Identifier of the article being paid for");
    opts.add(amazonOpts);
}


void payment::checkReturn( session& s, const char* page ) {
#if 0
    awsStandardButton button(s.valueOf("awsAccessKey"),
			     s.valueOf("awsSecretKey"),
			     s.valueOf("awsCertificate"));
#else
    payCheckoutButton button(url(s.valueOf("domainName")));
#endif	    
    if( !button.checkReturn(s,page) ) {
	throw std::runtime_error("wrong signature for request");
    }
}


void payment::fetch( session& s, const boost::filesystem::path& pathname ) {
    for( entrySeq::const_iterator e = entries.begin(); 
	 e != entries.end(); ++e ) {
	boost::smatch m;
	if( boost::regex_search(pathname.string(),m,*e->regexp) ) {
#if 0
	    awsStandardButton button(s.valueOf("awsAccessKey"),
				     s.valueOf("awsSecretKey"),
				     s.valueOf("awsCertificate"));
#else
	    payCheckoutButton button(url(s.valueOf("domainName")));
#endif	    
	    button.returnUrl = url(std::string("http://") 
				   + s.valueOf("domainName") 
				   + e->retPath);
	    article a = e->adapt->fetch(s,pathname);
	    button.description = a.descr;
	    button.build(a.guid,a.value);
	    button.writehtml(*ostr);    
	    break;
	}
    }
}


void payment::show( std::ostream& ostr,
		    session& s, const url& returnUrl, 
		    const std::string& referenceId, size_t value,
		    const std::string& descr )
{
#if 0
    awsStandardButton button(s.valueOf("awsAccessKey"),
			     s.valueOf("awsSecretKey"),
			     s.valueOf("awsCertificate"));
#else
    payCheckoutButton button(url(s.valueOf("domainName")));
#endif	    
    button.returnUrl = returnUrl;
    button.description = descr;
    button.build(referenceId,value);
    button.writehtml(ostr);    
}


void payPipeline::fetch( session& s, const boost::filesystem::path& pathname )
{
    std::stringstream r;
    r << s.valueOf("returnUrl") << "?referenceId=" << s.valueOf("referenceId");
    *ostr << httpHeaders.refresh(0,url(r.str())) << std::endl;
}    

