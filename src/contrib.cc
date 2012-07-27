/* Copyright (c) 2009-2011, Fortylines LLC
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

#include <iostream>
#include <sstream>
#include <boost/date_time.hpp>
#include <boost/system/error_code.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include "composer.hh"
#include "contrib.hh"
#include "markup.hh"
#include "mail.hh"
#include "auth.hh"


/** Pages related to contributors.

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/

#define LDAP_ENTRY(name,mode)								   \
	std::string name##Str = name.value(s);       \
    char *name##Vals[] = { (char *)name##Str.c_str(), NULL };	\
	LDAPMod name##Entry = { LDAP_MOD_##mode, #name, { name##Vals } }   \


namespace {

	char registerTemplate[] = "register";
	char registerMailTemplate[] = "register.eml";
	char registerConfirmTemplate[] = "register_confirm";
	char unregisterTemplate[] = "unregister";
	char passwordChangeTemplate[] = "password_change";
	char passwordResetTemplate[] = "password_reset";
	char passwordResetMailTemplate[] = "password_reset.eml";


    sessionVariable givenName("givenName","First Name");
	sessionVariable mail("mail","E-mail address");
    sessionVariable sn("sn","Last Name");
	sessionVariable oldPassword("oldPassword","old user password");


	void validate( const session& s ) {
#if 0		
		assert( !ldapAdmin.value(s).empty() );
		assert( !ldapPasswd.value(s).empty() );
#endif
	}

void modifyPassword( LDAPBound& ldapb, session& s, const std::string& who )
{
	LDAP *ld = ldapb._ld();

	LDAP_ENTRY(userPassword,REPLACE); /* userPassword, person (may) */
	LDAPMod *attrs[] = {
		&userPasswordEntry,
		NULL
	};

	/* ldap_modify_ext_s(LDAP *ld, char *dn, LDAPMod *mods[], 
	       LDAPControl **sctrls, LDAPControl **cctrls) */
	LDAP_NO_ERROR(ldapb,ldap_modify_ext_s(ld, who.c_str(), attrs, NULL, NULL));
}

void fetchLDAPProfile( LDAPBound& ldapb, session& s ) {

	std::string base = authContribDN(s,uid.value(s));
	int scope = LDAP_SCOPE_BASE;
	const char *filter = "(objectclass=*)";
	int attrsonly = 0; /* attribute types and values */
	LDAPMessage *res;
	const char *attrs[] = {
		mail.name,
		NULL
	};

    /* int ldap_search_ext_s( LDAP *ld, char *base, int scope, char *filter,
	       char *attrs[], int attrsonly, LDAPControl **serverctrls,
           LDAPControl **clientctrls, struct timeval *timeout,
           int sizelimit, LDAPMessage **res ); */
	LDAP *ld = ldapb._ld();
	LDAP_NO_ERROR(ldapb,ldap_search_ext_s(ld, base.c_str(), scope, filter,
										  (char**)attrs, attrsonly, 
										  NULL, NULL, NULL, -1, &res));

	BerElement *ber = NULL;
	for( LDAPMessage *msg = ldap_first_entry(ld,res); 
		 msg != NULL; msg = ldap_next_entry(ld,msg) ) {
		switch( ldap_msgtype(msg) ) {
		case LDAP_RES_SEARCH_ENTRY:  /* An Entry */
		case LDAP_RES_SEARCH_RESULT: /* (LDAP v3) */
			for( char *attr = ldap_first_attribute(ld,msg,&ber);
				 attr != NULL; attr = ldap_next_attribute(ld,msg,ber) ) {
				struct berval **vals = ldap_get_values_len(ld,msg,attr);
#if 0
				std::cerr << "\t" << attr << "=";
				int count = ldap_count_values_len(vals);
				const char *sep = "";
				for( int i = 0; i < count; ++i ) {
					std::cerr << sep << vals[i]->bv_val;
					sep = ", ";
				}
				std::cerr << std::endl;
#endif
				if( strcmp(attr,mail.name) == 0 ) {
					s.state(mail.name,std::string(vals[0]->bv_val));
				}
				ldap_value_free_len(vals);
			}
			break;
		}
	}
}

} // anonymous


contrib::pointer_type contrib::find( const std::string& email, 
									 const std::string& name )
{
	pointer_type p(new contrib());
	p->name = name;
	p->email = email;
	return p;
}


std::ostream& operator<<( std::ostream& ostr, const by& v ) {
	ostr << "by ";
	if( !v.ptr->email.empty() ) {
		ostr << html::a().href(std::string("mailto:") + v.ptr->email);
	}
	if( !v.ptr->name.empty() ) {
		ostr << v.ptr->name;
	} else if( !v.ptr->email.empty() ) {
		ostr << v.ptr->email;
	} else {
		ostr << "anonymous";
	}
	if( !v.ptr->email.empty() ) {
		ostr << "<sup><img src=\"/static/images/icon_email.png\"></sup>"
			 << html::a::end;
	}
	if( !v.ptr->google.empty() ) {
		ostr << " <a rel=\"author\" href=\"https://profiles.google.com/"
			 << v.ptr->google << "\">"
			 << "<img src=\"http://ssl.gstatic.com/images/icons/gplus-16.png\" width=\"16\" height=\"16\">"
			 << html::a::end;
	}
	return ostr;
}


std::ostream& operator<<( std::ostream& ostr, const from& v ) {
	if( !v.ptr->name.empty() ) {
		ostr << v.ptr->name << "<" << v.ptr->email << ">";
	} else {
		ostr << v.ptr->email;
	}
	return ostr;
}


void registerAddSessionVars( boost::program_options::options_description& all,
							 boost::program_options::options_description& visible )
{
    using namespace boost::program_options;
    
    options_description localOptions("registration");

    localOptions.add(mail.option());
    localOptions.add(givenName.option());
    localOptions.add(sn.option());
	localOptions.add(oldPassword.option());

    all.add(localOptions);
    visible.add(localOptions);
}


void registerEnter( session& s, const boost::filesystem::path& pathname )
{
	/* Prepare for next step in registration pipeline */
	std::stringstream next;
#if 0
	/* \todo Figure out how to store relative paths in pipeline
	   and absolute in e-mails. Use of decorator? */
	next << s.asAbsUrl(url(s.id()),"/confirm");
#else
	next << "/confirm/" << s.id();
#endif
	s.state(nextpage.name,next.str());
	s.store();

	/* Send e-mail to confirm proper email address. */
	sendMail(s,mail.value(s),"Register",registerMailTemplate);

	/* Prepare html page to invite registering user to check his/her inbox */
	compose<registerTemplate>(s,pathname);
}


void registerConfirm( session& s, const boost::filesystem::path& pathname ) {
	/* e-mail address confirmed, let's add the registered user into
	   the persistent database and show a confirmation webpage. */

	validate(s);

	std::string sessionId = pathname.leaf().string();
	s.loadsession(sessionId);
	
	/* add entry to LDAP database */
	LDAPBound ldapb(s,ldapAdmin.value(s),ldapAdminPassword);

	/* person
	     organizationalPerson
	       inetOrgPerson */
	char *objecClassVals[] = { "inetOrgPerson", NULL };
	LDAPMod objectClass = { LDAP_MOD_ADD, "objectClass", { objecClassVals } };
	LDAP_ENTRY(uid,ADD);          /* uid, (must) */
	LDAP_ENTRY(sn,ADD);           /* surname, person (must) */
	LDAP_ENTRY(mail,ADD);         /* e-mail address, inetOrgPerson (may) */
	LDAP_ENTRY(userPassword,ADD); /* userPassword, person (may) */

	/* common name, must have for person */
	std::stringstream cn;
	cn << givenName.value(s) << ' ' << sn.value(s); 
	std::string cnStr = cn.str();
	char *cnVals[] = { (char *)cnStr.c_str(), NULL };	
	LDAPMod cnEntry = { LDAP_MOD_ADD, "cn", { cnVals } };

	LDAPMod *attrs[] = {
		&objectClass,
		&uidEntry,
		&snEntry,
		&mailEntry,
		&userPasswordEntry,
		&cnEntry,
		NULL
	};

	std::string who = authContribDN(s,uid.value(s));
	LDAP *ld = ldapb._ld();
	LDAP_NO_ERROR(ldapb,ldap_add_ext_s(ld,who.c_str(), attrs, NULL, NULL));

	/* Write out thank you page. */
	compose<registerConfirmTemplate>(s,pathname);
}


void unregisterEnter( session& s, const boost::filesystem::path& pathname )
{
	std::string dn = authContribDN(s,uid.value(s));
	{
		/* Check contributors credential. */
		LDAPBound ldapb(s,dn,userPassword);
	}
	/* Bind with admin account since the contributor does not have sufficient
	   rights to delete its own key. */
	LDAPBound ldapb(s,ldapAdmin.value(s),ldapAdminPassword);
	LDAP_NO_ERROR(ldapb,ldap_delete_ext_s(ldapb._ld(),dn.c_str(),NULL,NULL));;
	compose<unregisterTemplate>(s,pathname);
}


void passwdChange( session& s, const boost::filesystem::path& pathname )
{
	LDAPBound ldapb(s,authContribDN(s,uid.value(s)),oldPassword);
	modifyPassword(ldapb,s,authContribDN(s,uid.value(s)));

	compose<passwordChangeTemplate>(s,pathname);
}


void passwdReset( session& s, const boost::filesystem::path& pathname )
{
	std::string chars(
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "1234567890"
        "!@#$%^&*()~-_=+[{]{\\|;:'\",<.>/?");
    boost::random::random_device rng;
    boost::random::uniform_int_distribution<> index_dist(0, chars.size() - 1);
	std::stringstream newPassword;
    for( int i = 0; i < 8; ++i ) {
        newPassword << chars[index_dist(rng)];
    }

	LDAPBound ldapb(s,ldapAdmin.value(s),ldapAdminPassword);

	/* We must fetch at the email address in order to send an email
	   with the new password. */
	fetchLDAPProfile(ldapb,s);
	s.state(userPassword.name,newPassword.str());
	modifyPassword(ldapb,s,authContribDN(s,uid.value(s)));

	sendMail(s,mail.value(s),"Password Reset",passwordResetMailTemplate);
	compose<passwordResetTemplate>(s,pathname);
}


void contribIdxFetch( session& s, const boost::filesystem::path& pathname )
{
    using namespace std;
    using namespace boost::filesystem;
    
#if 0
    s.out() << html::table();    
#endif
    if( is_directory(pathname) ) {
	for( directory_iterator entry = directory_iterator(pathname); 
	     entry != directory_iterator(); ++entry ) {
	    if( is_directory(*entry) ) {
		boost::filesystem::path 
		    profilePathname = path(*entry) / "profile.blog";	
		if( boost::filesystem::is_regular_file(profilePathname) ) {
#if 0
		    s.out() << html::tr() 
			 << html::td() 
			 << "<img src=\"/resources/contrib/" 
			 << entry->filename() << "/profile.jpg\">"
			 << html::td::end
			 << html::td();
#endif		    
		    textFetch(s,profilePathname);
#if 0		    
		    s.out() << html::td::end
			 << html::tr::end;
#endif
		}
	    }
	}
    }
#if 0
    s.out() << html::table::end;
#endif
}


