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

#include <iostream>
#include <sstream>
#include <boost/date_time.hpp>
#include <boost/system/error_code.hpp>
#include <boost/filesystem/fstream.hpp>
#include "composer.hh"
#include "contrib.hh"
#include "markup.hh"
#include "auth.hh"


/** Pages related to contributors.

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


void contrib::normalize() {
    name = ::normalize(name);
    email = ::normalize(email);
}

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
    if( !v.ptr->name.empty() ) {
        ostr << v.ptr->name;
    } else if( !v.ptr->email.empty() ) {
        ostr << v.ptr->email;
    } else {
        ostr << "anonymous";
    }
    return ostr;
}

std::ostream& operator<<( std::ostream& ostr, const contact& v ) {
    if( !v.ptr->email.empty() ) {
        ostr << html::a().href(std::string("mailto:") + v.ptr->email);
        ostr << "<img src=\"/static/img/email-32.jpg\" width=\"32\" height=\"32\">"
             << html::a::end;
    }
    if( !v.ptr->linkedin.empty() ) {
        ostr << " <a rel=\"author\" href=\"http://www.linkedin.com/pub/"
             << v.ptr->linkedin << "\">"
             << "<img src=\"/static/img/linkedin-32.png\" width=\"32\" height=\"32\">"
             << html::a::end;
    }
    if( !v.ptr->google.empty() ) {
        ostr << " <a rel=\"author\" href=\"https://profiles.google.com/"
             << v.ptr->google << "\">"
             << "<img src=\"/static/img/gplus-32.png\" width=\"32\" height=\"32\">"
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


void contribIdxFetch( session& s, const url& name )
{
    using namespace std;
    using namespace boost::filesystem;

#if 0
	/* XXX re-enable when we have actual contributor profiles */
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
#endif
}


