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

#ifndef guardcalendar
#define guardcalendar

#include "document.hh"
#include "rfc5545tok.hh"

/**
   Display a calendar as an HTML page.
   See: iCalendar - RFC5545 (http://tools.ietf.org/html/rfc5545)

   Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/

namespace tero {

extern sessionVariable month;

void
calendarAddSessionVars( boost::program_options::options_description& all,
    boost::program_options::options_description& visible );


class calendar : public rfc5545TokListener {
protected:
    typedef void
    (calendar::* walkNodePtr)( const std::string& s ) const;

    struct walkNodeEntry {
        const char* name;
        walkNodePtr start;
        walkNodePtr end;

        bool operator<( const walkNodeEntry& right ) const;
    };

    static walkNodeEntry walkers[];

    void any( const std::string& s ) const;
    walkNodeEntry* walker( const std::string& s ) const;

public:
    virtual void newline(const char *line, int first, int last );

    virtual void token( rfc5545Token token, const char *line,
        int first, int last, bool fragment );

};


void calendarFetch( session& s, std::istream& in,
    const url& name );

}

#endif
