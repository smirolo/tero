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

#ifndef guardinvoices
#define guardinvoices

#include <exception>
#include "document.hh"

class statement : public document {
protected:

    typedef std::vector<std::pair<boost::posix_time::ptime,
				 boost::posix_time::ptime> > hourSet;
    struct contract {
	int tarif;
	std::string descr;
	std::string address;
	hourSet hours;

	contract() : tarif(1) {}
    };

    typedef std::map<std::string,contract> timesMap;

    timesMap billed;

    /** Load information associated to the contracts 
	for which an invoice exists.
    */
    void contracts( const boost::filesystem::path& db );

    void header();

    void footer();

    void timeRange( const std::string& msg,
		    const boost::posix_time::ptime start,
		    const boost::posix_time::ptime stop );

public:
    virtual void fetch( session& s, const boost::filesystem::path& pathname );
};


#endif