/* Copyright (c) 2009-2013, Fortylines LLC
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

#ifndef guardauth
#define guardauth

#include <ldap.h>
#include <exception>
#include "document.hh"

/* This module implements <a href="/reps/whitepapers/doc/access.book">authentication</a> and de-authentication of users.

   It relies on outside authentication services to do the  heavy duty work,
   PAM and LDAP for now.

   Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


/** session variable for a contributor's account unique identifier.
*/
extern sessionVariable uid;

/** session variable for a contributor's password.
*/
extern sessionVariable userPassword;


/** Exception thrown when there is a problem authenticating a contributor
    with a (*uid*, *userPassword*) pair.
*/
class invalidAuthentication : public std::runtime_error {
public:

   enum reason {
       REASON_EMPTY_UID        = 0,
       REASON_EMPTY_PASSWORD,
       REASON_ALL_METHOD_FAILED,
       REASON_PAM_START,
       REASON_PAM_AUTHENTICATE,
       REASON_LDAP_AUTHENTICATE,
       REASON_LAST_REASON
   };

    static const char* message( const reason& r );

    explicit invalidAuthentication( const reason& r )
        : std::runtime_error(message(r)) {}

};


/* LDAP session variables. */
extern urlVariable ldapHost;
extern intVariable ldapPort;
extern sessionVariable ldapDN;
extern sessionVariable ldapAdmin;
extern sessionVariable ldapAdminPassword;


/** Exception thrown when an error is returned by a LDAP library call.
 */
class LDAPException : public std::runtime_error {
public:
    explicit LDAPException( const std::string& w )
        : std::runtime_error(w) {}
};


/** Resource to bind to a LDAP server. The constructor does
    the binding and the destructor does the unbinding.
 */
class LDAPBound {
private:
    LDAP *ld;

public:
    LDAPBound( const session& s,
        const std::string& who,
        const sessionVariable& password );

    ~LDAPBound();

    /** Throws an LDAPException if *err* is non-zero. It will also add
        additional information associated with the last LDAP error.
    */
    void assertNoError( int err, const char* file, int line );

    LDAP *_ld() const { return ld; }
};

/* Helper macro useful to check error code returned by an LDAP API call. */
#define LDAP_NO_ERROR(ldapb,calls) \
    (ldapb).assertNoError(calls,__FILE__,__LINE__)


std::string authContribDN( const session& s, const std::string& uid );


/** Add session variables related to this auth module.
 */
void authAddSessionVars( boost::program_options::options_description& opts,
    boost::program_options::options_description& visible );


/** Web-based authentication checks the credentials passed to the CGI
    against permissions granted on the server and returns a session
    identifier saved as a cookie in the browser on the client-side
    for further requests.
*/
void loginFetch( session& s, const url& name );


/** Forced de-authentication will invalidate the current session identifier
    passed as part of the request.
*/
void logoutFetch( session& s, const url& name );

#endif
