/* Copyright (c) 2009-2012, Fortylines LLC
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

#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <boost/date_time.hpp>
#include <boost/system/error_code.hpp>
#include <boost/filesystem/fstream.hpp>
#include "auth.hh"
#include "composer.hh"
#include <security/pam_appl.h>


/** Authentication through PAM.

	Useful urls:
	<a href="http://www.freebsd.org/doc/en/articles/pam/pam-essentials.html">FreeBSD PAM Essentials</a>
    <a href="http://linux.die.net/man/3/pam">PAM Linux man page</a>
    <a href="http://www.linuxjournal.com/article/5940"></a>

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


sessionVariable uid("uid","uid");
sessionVariable userPassword("userPassword","password");

urlVariable ldapHost("ldapHost",
					 "LDAP host to connect to");
intVariable ldapPort("ldapPort",
					 "LDAP port on host to connect to");
sessionVariable ldapDN("ldapDN",
	 "DN to which contributors are added (ex. ou=people,dc=example,dc=com)");


    sessionVariable ldapAdmin("ldapAdmin",
	   "admin with write access to DN (ex. cn=admin,dc=example,dc=com");
    sessionVariable ldapAdminPassword("ldapAdminPassword","Password for the LDAP admin");


static int su_conv( int num_msg,
					const struct pam_message **msgm,
					struct pam_response **resp,
					void *appdata );

namespace {

	void validate( const session& s ) {
		assert( !ldapHost.value(s).empty() );
		assert( ldapPort.value(s) != 0);
		assert( !ldapDN.value(s).empty() );
	}


boost::posix_time::time_duration stop( session& s ) {
    using namespace boost::system;
    using namespace boost::posix_time;
    using namespace boost::filesystem;

    ptime start = startTime.value(s);
    ptime stop = second_clock::local_time();
    time_duration aggregate = stop - start;
    
#if 0
    ofstream file;
    s.appendfile(file,contributorLog(s));
    std::string msg = authMessage.value(s);
    if( !msg.empty() ) {
	file << msg << ':' << std::endl;
    }
    file << start << ' ' << stop << std::endl;
    file.close();
#endif
    s.remove();

    return aggregate;
}

/** Authentication using PAM.
*/
void authPAM( session& s ) {
    struct pam_conv conv = {
        su_conv,    /* function pointer to the conversation function */
        &s          /* callback parameter */
    };

    /* Call pam_start(service, user, pam_conv, handle) */
    pam_handle_t *pamh = NULL;
    int retval =  pam_start("semilla",NULL,&conv,&pamh);
    if( retval != PAM_SUCCESS ) {
		throw invalidAuthentication(invalidAuthentication::REASON_PAM_START);
    }

    /* Call pam_authenticate(handle, flags) */
    retval = pam_authenticate(pamh,0);
    if( retval != PAM_SUCCESS ) {
		throw invalidAuthentication(invalidAuthentication::REASON_PAM_AUTHENTICATE);
    }

    int pam_status = 0; 
    if( pam_end(pamh,pam_status) != PAM_SUCCESS ) { 
		pamh = NULL;
    }
}


/** Authentication using LDAP.
*/
void authLDAP( const session& s ) {
	try {
		LDAPBound ldapb(s,authContribDN(s,uid.value(s)),userPassword);
	} catch( LDAPException& except ) {
		throw invalidAuthentication(invalidAuthentication::REASON_LDAP_AUTHENTICATE);
	}
}

} // anonymous namespace


const char* invalidAuthentication::message( const reason& r ) {
	static const char *messages[] = {
		"invalidAuthentication (EMPTY_UID)",
		"invalidAuthentication (EMPTY_PASSWORD)",
		"invalidAuthentication (ALL_METHOD_FAILED)",
		"invalidAuthentication (PAM_START)",
		"invalidAuthentication (PAM_AUTHENTICATE)"
		"invalidAuthentication (LDAP_AUTHENTICATE)"
	};
	if( r < REASON_LAST_REASON ) {
		return messages[r];
	}
	return "invalidAuthentication (UNKNOWN)";
}


static int su_conv( int num_msg,
					const struct pam_message **msgm,
					struct pam_response **resp,
					void *appdata )
{
    session *s = (session*)appdata;
    
    int count;
    struct pam_response *r;   
    r = (struct pam_response*)calloc(num_msg,sizeof(struct pam_response));
	
    for( count=0; count < num_msg; ++count) {	
		char *value = NULL;
		switch(msgm[count]->msg_style) {
		case PAM_PROMPT_ECHO_OFF:
			value = (char*)malloc((userPassword.value(*s).size() + 1)
								  * sizeof(char));
			strcpy(value,userPassword.value(*s).c_str());
			break;
		case PAM_PROMPT_ECHO_ON:
			value = (char*)malloc((uid.value(*s).size() + 1)
								  * sizeof(char));
			strcpy(value,uid.value(*s).c_str());
			break;	    
		case PAM_ERROR_MSG:
		case PAM_TEXT_INFO:
			std::cerr << msgm[count]->msg;
			break;
		default:
			std::cerr << "Erroneous Conversation (" 
					  << msgm[count]->msg_style << ")" << std::endl;
		}
		r[count].resp_retcode = 0;
		r[count].resp = value;
    }
    *resp = r;
    return PAM_SUCCESS;
}


LDAPBound::LDAPBound( const session& s,
					  const std::string& who,
					  const sessionVariable& password )
{
	validate(s);

	std::string host = ldapHost.value(s).string();
	int port = ldapPort.value(s);
	if( port <= 0 ) port = LDAP_PORT;

	std::stringstream uri;
	uri << "ldap://" << host.c_str() << ":" << port;
	LDAP_NO_ERROR(*this,ldap_initialize(&ld,uri.str().c_str()));

	int protocol = 3;
	LDAP_NO_ERROR(*this,ldap_set_option(ld,LDAP_OPT_PROTOCOL_VERSION,&protocol));

	std::string passwd = password.value(s);
#if 0
	std::cerr << "!!! [auth] " << who.c_str()
			  << ", passwd: '" << passwd << "'" << std::endl;
#endif

	/* ldap_simple_bind_s has been deprecated thus we use ldap_sasl_bind_s
	   with simple credentials. Here is the prototype for it:
	   ldap_sasl_bind_s( LDAP *ld, const char *dn, const char *mechanism,
	                     struct berval *cred, LDAPControl *sctrls[],
						 LDAPControl *cctrls[], struct berval **servercredp) */
	berval cred;
	cred.bv_val = (char *)passwd.c_str();
	cred.bv_len = (size_t) passwd.size();
	const char *mechanism = (const char *)LDAP_SASL_SIMPLE;
	LDAP_NO_ERROR(*this,ldap_sasl_bind_s(ld,who.c_str(),mechanism,&cred,
										 NULL,NULL,NULL));
}


std::string authContribDN( const session& s, const std::string& uid ) {
	std::stringstream who;
	who << "uid=" << uid << ',' << ldapDN.value(s);
	return who.str();
}


LDAPBound::~LDAPBound()
{
	int err;
	if( (err = ldap_unbind_ext_s(ld, NULL, NULL)) ) {
		std::stringstream str;
		str << "ldap_unbind_ext_s: " << ldap_err2string(err);
		throw LDAPException(str.str());
	}
}


void LDAPBound::assertNoError( int err, const char* file, int line )
{
	if( err ) {
		char *info = NULL;
		std::stringstream str;
		str << file << ':' << line << ": " << ldap_err2string(err);
		ldap_get_option(ld,LDAP_OPT_DIAGNOSTIC_MESSAGE,&info);
		if( info && strlen(info) > 0 ) {
			str << ", additionnal info: " << info;
		}
		ldap_memfree(info);
		boost::throw_exception(LDAPException(str.str()));
	}
}


void authAddSessionVars( boost::program_options::options_description& all,
						 boost::program_options::options_description& visible )
{
    using namespace boost::program_options;
    
    options_description localOptions("authentication");
    localOptions.add(uid.option());
    localOptions.add(userPassword.option());

	localOptions.add(ldapHost.option());
	localOptions.add(ldapPort.option());
	localOptions.add(ldapDN.option());

	localOptions.add(ldapAdmin.option());
	localOptions.add(ldapAdminPassword.option());

    all.add(localOptions);
    visible.add(localOptions);
}


void loginFetch( session& s, const boost::filesystem::path& pathname ) {
    using namespace boost::system;
    using namespace boost::posix_time;
    using namespace boost::filesystem;
    
    if( uid.value(s).empty() ) {
		throw invalidAuthentication(invalidAuthentication::REASON_EMPTY_UID);
	}

	if( userPassword.value(s).empty() ) {
		throw invalidAuthentication(invalidAuthentication::REASON_EMPTY_PASSWORD);
    }

	/* Actual authentication using multiple service in order. */
	bool authenticated = false;
	if( !authenticated ) {
		try {
			authPAM(s);
			authenticated = true;
		} catch( invalidAuthentication& invalid ) {
		}
	}
	if( !authenticated ) {
		try {
			authLDAP(s);
			authenticated = true;
		} catch( invalidAuthentication& invalid ) {
		}
	}
	if( !authenticated ) {
		throw invalidAuthentication(invalidAuthentication::REASON_ALL_METHOD_FAILED);
	}

    /* The authentication will create a persistent ("uid",value)
       pair that, in turn, will create a sessionId and a startTime. */
    s.state(uid.name,uid.value(s));
    s.store();
    
    /* Set a session cookie and redirect to orignal page requested. */
    httpHeaders.setCookie(s.sessionName,
						  s.id()).location(url(document.value(s).string()));
	if( !nextpage.value(s).empty() ) {
		httpHeaders.refresh(0,nextpage.value(s));
	}
}


char logoutTemplate[] = "logout";

void logoutFetch( session& s, const boost::filesystem::path& pathname ) {
    using namespace boost::system;
    using namespace boost::posix_time;
    using namespace boost::filesystem;

    if( s.exists() ) {
		httpHeaders.setCookie(s.sessionName,s.id(),
							  boost::posix_time::ptime::date_duration_type(-1));

		time_duration logged = stop(s);
		std::stringstream logstr;
		logstr << logged.hours() << " hours logged." << std::endl;
		s.state("hours",logstr.str());    
		compose<logoutTemplate>(s,document.name);
	}
}


