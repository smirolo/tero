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
#include "payment.hh"
#include "markup.hh"
#include <boost/filesystem/fstream.hpp>

/** Tero processing pipeline

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


namespace {

    void showConfig( std::ostream& ostr,
		     const std::string& domainName, 
		     const std::string& adminLogin ) {
	ostr << html::p()
	     << "The configuration package will use the following information:"
	     << html::p::end;    
    
	ostr << html::table()
	     << html::tr()
	     << html::td() << "domainName" << html::td::end
	     << html::td() << domainName << html::td::end
	     << html::tr::end
	     << html::tr()
	     << html::td() << "adminLogin" << html::td::end
	     << html::td() << adminLogin << html::td::end
	     << html::tr::end
	     << html::table::end;

	ostr << html::p() << html::p::end;
    }

}  // anonymous namespace


void confgenCheckout::addSessionVars( 
			  boost::program_options::options_description& opts ) {
    using namespace boost::program_options;
    options_description teroOpts("tero");
    teroOpts.add_options()
	("confgenDomain",value<std::string>(),"confgenDomain")
	("confgenAdmin",value<std::string>(),"confgenAdmin");
    opts.add(teroOpts);
}


void 
confgenCheckout::meta( session& s, const boost::filesystem::path& pathname ) 
{
    s.vars["title"] = "fortylines tero checkout";
}

void 
confgenCheckout::fetch( session& s, const boost::filesystem::path& pathname ) 
{
    /* Build the *referenceId* that will be used to generate the *packageName*
       as part of the delivery step and generate a meaningful description 
       that will be displayed in the Amazon checkout page. */
    std::string domainName = s.valueOf("confgenDomain");
    std::string adminLogin = s.valueOf("confgenAdmin");
    std::stringstream referenceId;
    referenceId << s.valueOf("confgenAdmin") << '@' << domainName;
    std::stringstream descr;
    descr << "config of " << domainName << " as a dev server";

    /* Print the HTML that explains the product sold 
       alongside the pay button. */
    *ostr << html::p()
	  << "For $25, you will be able to downlaod a package that configures"
	" a stock Ubuntu 10.04 Server Edition (ubuntu-10.04-server-amd64.iso)"
	" to serve as a communication platform for your development team."
	  << html::p::end;
    
    *ostr << "This includes:"
	  << html::ul()
	  << html::li() << "A website for " << domainName
	  << html::ul()
	  << html::li() << "that supports http and https"
	  << html::li() << "with a source repository browser"
	  << html::li() << "with a todo tracking system"
	  << html::ul::end
	  << html::ul()
	  << html::li() << "A SMTP mail server for " << domainName
	  << html::li() << "with the following mailing list: dev@" << domainName
	  << html::ul::end
	  << "And, only accessible through a ssh tunnel,"
	  << html::ul()
	  << html::li() << "An IMAP mail account"
	  << html::li() << "A private http server"
	  << html::ul()
          << html::li() << "for mailing list administration"
          << html::li() << "for web tracking statistics"
	  << html::ul::end
	  << html::ul::end;
    
    showConfig(*ostr,domainName,adminLogin);

    /* Be careful to use our domainName here and not the domainName
       being configured. */
    payment::show(*ostr,s,
		  url("http",s.valueOf("domainName"),nextPathname),
		  referenceId.str(),25,descr.str());
    
    *ostr << html::p() 
	  << "If you are not convinced yet, feel free to keep browsing"
	" around fortylines website. This server is running"
	" the exact platform you will enjoy. As a matter of fact, we tear"
	" our entire infrastructure down and put it back together on a regular"
	" basis to insure the highest level of simplicity and reliability."
	  << html::p::end;
}


void 
confgenDeliver::meta( session& s, const boost::filesystem::path& pathname )
{
    /* Extract the *domainName* and *adminLogin* that will be used to generate
       the configuration package.*/
    std::string referenceId = s.valueOf("referenceId");
#if 0
    if( referenceId.empty() ) {
	/* Hack used for testing */
	referenceId = "adm@codespin.is-a-geek.com";
    }
#endif
    adminLogin = referenceId.substr(0,referenceId.find('@'));
    domainName = referenceId.substr(referenceId.find('@') + 1);
    std::string packageName = domainName;
    std::replace(packageName.begin(),packageName.end(),'.','-');
    packagePath = s.valueOf("buildTop") 
	+ "/" + packageName + "_0.1" + "-ubuntu1_amd64.deb";

    std::stringstream d;
    d << "tero package for " << domainName;
    s.vars["title"] = d.str();

    d.str("");
    d << "/download/" << packagePath.filename();
    httpHeaders.refresh(5,url(d.str()));
}


void 
confgenDeliver::fetch( session& s, const boost::filesystem::path& pathname ) 
{
    /* Check the request is actually coming from Amazon. */
    payment::checkReturn(s,thisPathname);

    /* Print a thank you note and the instruction to install the package
       on the server machine. */
    *ostr << html::p()
	  << "Thank you for your kind business. The download of your personal"
	  << " configuration package should start shortly. You should execute"
	  << " the following command to install it onto your machine"
	  << html::p::end;

    *ostr << html::pre().classref("code")
	  << "DEBIAN_FRONTEND=noninteractive /usr/bin/apt-get -y install \\\n"
       "     postfix mailman dovecot-imapd apache2 awstats libsasl2-2 sasl2-bin"
	  << std::endl
	  << "sudo dpkg -i --force-overwrite " << packagePath.filename()
	  << html::pre::end;

    *ostr << html::p()
	  << "In order to verify the configuration is working correctly,"
	  << " you can then follow a " << html::a().href("testing.book")
	  << "few basic steps" << html::a::end << '.'
	  <<  html::p::end;

    showConfig(*ostr,domainName,adminLogin);

    *ostr << html::p()
	  << "For any comment, please feel free to "
	  << html::a().href("mailto:info@fortylines.com") 
	  << "contact us" << html::a::end << '.'
	  << html::p::end;

    /* Execute the underlying script used to generate 
       the configuration package. */
    int err = 0;
    std::stringstream cmd;
    cmd << s.valueOf("binDir") << "/dservices --skip-recurse " 
	<< domainName << " " << adminLogin;

#if 0
    err = system(cmd.str().c_str());
#else
    char line[256];
    FILE *cmdfile = popen(cmd.str().c_str(),"r");
    if( cmdfile == NULL ) {
	throw std::runtime_error("error: unable to execute command.");
    }
    while( fgets(line,sizeof(line),cmdfile) != NULL ) {
	std::cerr << line;
    }
    err = pclose(cmdfile);
#endif

    if( err ) {
	throw std::runtime_error("error generating the configuration file");
    }
}


void 
forceDownload::fetch( session& s, const boost::filesystem::path& pathname ) 
{    
    boost::filesystem::path packagePath = s.abspath(pathname);

    size_t packageSize = boost::filesystem::file_size(packagePath);

    *ostr << httpHeaders.contentType("application/octet-stream","")
	.contentLength(packageSize)
	.contentDisposition("attachment",pathname.filename());

    boost::filesystem::ifstream istr(packagePath,std::ios::binary);
    char conf[packageSize];
    istr.read(conf,packageSize);
    ostr->write(conf,packageSize);
    istr.close();
}

