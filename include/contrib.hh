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

#ifndef guardcontrib
#define guardcontrib

#include "document.hh"
#include <boost/tr1/memory.hpp>

/** Pages related to contributors such as registration of new contributors,
	profile management, password updates, etc.

   Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/

class contrib {
public:
	typedef std::tr1::shared_ptr<contrib> pointer_type;

public:
	std::string email;
	std::string name;
	std::string google;
	
public:
	static pointer_type find( const std::string& email, 
							  const std::string& name = "" );
};


class by {
protected:
	const contrib::pointer_type& ptr;

public:
	friend std::ostream& operator<<( std::ostream&, const by& );

	explicit by( const contrib::pointer_type& p ) : ptr(p) {}
};

class from {
protected:
	const contrib::pointer_type& ptr;

public:
	friend std::ostream& operator<<( std::ostream&, const from& );

	explicit from( const contrib::pointer_type& p ) : ptr(p) {}
};

/** Add session variables related to auth module.
 */
void registerAddSessionVars( boost::program_options::options_description& opts,
						 boost::program_options::options_description& visible );


/** Index the set of contribs and display an HTML row per contrib item
    that contains the date, author and title of the contrib item.
 */
void contribIdxFetch( session& s, const boost::filesystem::path& pathname );


/** Reply to POST request to create a new contributor account. It creates
	a session identifier and sends an email to the contributor.
 */
void registerEnter( session& s, const boost::filesystem::path& pathname );


/** Last stage in the creation a new contributor account.
 */
void registerConfirm( session& s, const boost::filesystem::path& pathname );


/** Even if we delete a contributor's ability to log in, we still want
	to keep all references to their name and email address in the repository
	commits. */
void unregisterEnter( session& s, const boost::filesystem::path& pathname );


/** Reply to POST request to change a contributor's password.
 */
void passwdChange( session& s, const boost::filesystem::path& pathname );


/** Modify a contributor's password to a randomly generated password
	and e-mail that password to the contributor.
*/
void passwdReset( session& s, const boost::filesystem::path& pathname );

#endif
