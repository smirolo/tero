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

#ifndef guardauth
#define guardauth

#include <exception>
#include "document.hh"

/* Contributors are <a href="/tero/doc/access.book">authenticated</a> before
   any write operation is performed. 

   Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/

class invalidAuthentication : public std::exception {
public:

    virtual const char* what() const throw() {
	return "invalidAuthentication";
    }
};


class auth : public document {
public:
    static void 
    addSessionVars( boost::program_options::options_description& opts );

    virtual void fetch( session& s, const boost::filesystem::path& pathname ) const;
};

/** Web-based authentication checks the credentials passed to the CGI
    against permissions granted on the server and returns a session 
    identifier saved as a cookie in the browser on the client-side 
    for further requests. 
*/
class login : public auth {
public:
    virtual void fetch( session& s, const boost::filesystem::path& pathname ) const;
};


class deauth : public document {
protected:
     /* print out monthly aggregates and return last one. */
    boost::posix_time::time_duration aggregate( const session& s ) const;

    /* stop counter and return session time. */
    boost::posix_time::time_duration stop( session& s ) const;

public:
    virtual void fetch( session& s, const boost::filesystem::path& pathname ) const;
};


/** Force de-authentication will invalidate the current session identifier
    passed as part of the request.
*/
class logout : public deauth {
public:
    virtual void fetch( session& s, const boost::filesystem::path& pathname ) const;
};

#endif
