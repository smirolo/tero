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

#include "mails.hh"
#include "markup.hh"


void post::normalize() {
    title = ::normalize(title);
    author = ::normalize(author);
}

void post::expanded( std::ostream& ostr ) const {
    ostr << html::h(1) << title << html::h(1).end();
    ostr << html::h(2) << author << html::h(2).end();
    ostr << html::h(2) << time << html::h(2).end();
    ostr << html::p() << descr << html::p::end;	 
    ostr << std::endl;
}

void oneliner::filters( const post& p ) {
    std::cout << html::tr() 
	      << html::td() << p.time << html::td::end
	      << html::td() << p.author << html::td::end
	      << html::td() << p.title << html::td::end;
    if( false ) {
	std::cout << html::td() 
		  << html::a().href("/vote")
		  << "<img src=\"/resources/amazon.png\">"
		  << html::a::end
		  <<  html::td::end;
    }
    std::cout << html::tr::end;
}

void blogwriter::filters( const post& p ) {
    *ostr << "Title: " << p.title << std::endl;
    *ostr << "Date: " << p.time << std::endl;
    *ostr << "Author: " << p.author << std::endl;
    *ostr << std::endl << std::endl;
    *ostr << p.descr << std::endl;
}