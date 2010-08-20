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

#include "payment.hh"
#include "todo.hh"
#include "aws.hh"

void payment::add( const boost::regex& r, 
		   const char *retPath, adapter& a ) {
    entries.push_back(entry(r,retPath,a));
}


void 
payment::addSessionVars( boost::program_options::options_description& opts ) {
    using namespace boost::program_options;
    /* For authentication with Amazon payment services. */
    opts.add_options()
	("awsAccessKey",value<std::string>(),"Amazon Access Key")
	("awsSecretKey",value<std::string>(),"Amazon Secret Key")
	("awsCertificate",value<std::string>(),"Amazon Public Certificate");
}


void payment::checkReturn( session& s, const char* page ) {
    awsStandardButton button(s.valueOf("awsAccessKey"),
			     s.valueOf("awsSecretKey"),
			     s.valueOf("awsCertificate"));
    if( !button.checkReturn(s,page) ) {
	throw std::runtime_error("wrong signature for request");
    }
}


void payment::fetch( session& s, const boost::filesystem::path& pathname ) {
    for( entrySeq::const_iterator e = entries.begin(); 
	 e != entries.end(); ++e ) {
	boost::smatch m;
	if( boost::regex_search(pathname.string(),m,*e->regexp) ) {
	    awsStandardButton button(s.valueOf("awsAccessKey"),
				     s.valueOf("awsSecretKey"),
				     s.valueOf("awsCertificate"));
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
    awsStandardButton button(s.valueOf("awsAccessKey"),
			     s.valueOf("awsSecretKey"),
			     s.valueOf("awsCertificate"));
    button.returnUrl = returnUrl;
    button.description = descr;
    button.build(referenceId,value);
    button.writehtml(ostr);    
}
