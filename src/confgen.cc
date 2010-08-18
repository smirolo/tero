/* Copyright (c) 2010, Fortylines LLC
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

#include "confgen.hh"
#include "aws.hh"

void confgenCheckout::addSessionVars( 
			  boost::program_options::options_description& opts ) {
    using namespace boost::program_options;
    opts.add_options()
	("confgenDomain",value<std::string>(),"confgenDomain")
	("confgenAdmin",value<std::string>(),"confgenAdmin");
}


void 
confgenCheckout::fetch( session& s, const boost::filesystem::path& pathname ) 
{
    awsStandardButton button(s.valueOf("awsAccessKey"),
			     s.valueOf("awsSecretKey"),
			     s.valueOf("awsCertificate"));
    button.returnUrl = url(std::string("http://") 
			   + s.valueOf("domainName") 
			   + "/teroDeliver");

    *ostr << "<p>domainName: " << s.valueOf("confgenDomain") << "</p>";
    *ostr << "<p>adminName: " << s.valueOf("confgenAdmin") << "</p>";

    std::stringstream id;
    id << s.valueOf("confgenAdmin") << '@' << s.valueOf("confgenDomain");
    button.description = "tero config";
    button.build(id.str(),25);
    button.writehtml(*ostr);    
}


void 
confgenDeliver::fetch( session& s, const boost::filesystem::path& pathname ) 
{
    awsStandardButton button(s.valueOf("awsAccessKey"),
			     s.valueOf("awsSecretKey"),
			     s.valueOf("awsCertificate"));
    if( !button.checkReturn(s,"/teroDeliver") ) {
	throw std::runtime_error("wrong signature for request");
    }

    std::string referenceId = s.valueOf("referenceId");
    std::string confgenAdmin = referenceId.substr(0,referenceId.find('@'));
    std::string confgenDomain = referenceId.substr(referenceId.find('@'));

    *ostr << "<p>Generate configuration for:</p>";
    *ostr << "<p>domainName: " << confgenDomain << "</p>";
    *ostr << "<p>adminName: " << confgenAdmin << "</p>";

    char lcstr[256];
    std::stringstream cmd;
    cmd << s.valueOf("binDir") << "/dservices " 
	<< confgenDomain << " " << confgenAdmin;
    FILE *summary = popen(cmd.str().c_str(),"r");
    assert( summary != NULL );   
    while( fgets(lcstr,sizeof(lcstr),summary) != NULL ) {
	*ostr << lcstr;
    }
    pclose(summary);
 
}
