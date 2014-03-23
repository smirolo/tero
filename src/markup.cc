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

#include "markup.hh"
#include "Poco/DateTimeParser.h"

/** Markups.

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/

namespace tero {

/* \todo review memory usage for this function. */
std::string strip( const std::string& s ) {
    const char *seps = " \t\n\r";
    std::string::size_type first = s.find_first_not_of(seps);
    return ( first == std::string::npos ) ? std::string()
        : s.substr(first,s.find_last_not_of(seps) - first + 1);
}

std::string normalize( const std::string& s ) {
    std::string result;
    size_t p = 0;
    while( p < s.size() && isspace(s[p]) ) ++p;
    while( p < s.size() ) {
        if( isspace(s[p]) ) {
            while( p < s.size() && isspace(s[p]) ) ++p;
            if( p < s.size() ) result += ' ';
        } else {
            result += s[p++];
        }
    }
    return result;
}

/**
   returns a datetime as UTC.
--date=iso (or --date=iso8601) shows timestamps in ISO 8601
           format.

boost::posix_time::ptime:
"Defines a non-adjusted time system with nano-second/micro-second resolution and stable calculation properties."

 */
boost::posix_time::ptime parse_datetime( const std::string& str ) {
    int timeZoneDifferential = 0;
    Poco::DateTime parsed = Poco::DateTimeParser::parse(str, timeZoneDifferential);
    /* From the Poco documentation:
       "The date and time stored in a DateTime is always in UTC"
       "A Timestamp stores a monotonic* time value with (theoretical)
       microseconds resolution."
       "std::time_t TimeStamp::epochTime() const returns the timestamp
       expressed in time_t. time_t base time is midnight, January 1, 1970.
       Resolution is one second."
       C++ Reference:
       "For historical reasons, it is generally implemented as an integral
       value representing the number of seconds elapsed since 00:00 hours,
       Jan 1, 1970 UTC."
       From boost documentation:
       "boost::posix_time::ptime defines a non-adjusted time system with
       nano-second/micro-second resolution and stable calculation properties."
       "ptime::from_time_t creates a ptime from the time_t parameter.
       The seconds held in the time_t are added to a time point of 1970-Jan-01."
     */
    return boost::posix_time::from_time_t(
        parsed.timestamp().epochTime());
}


std::string extractEmailAddress( const std::string& line ) {
    size_t estart = line.find('<');
    size_t efinish = line.rfind('>');
    if( estart < efinish ) {
        return line.substr(estart + 1,efinish - estart - 1);
    }
    return line;
}

std::string extractName( const std::string& line ) {
    size_t estart = line.find('<');
    if( estart != std::string::npos ) {
        return strip(line.substr(0,estart));
    }
    return line;
}


namespace html {

    const char* a::name = "a";
    const detail::nodeEnd a::end(a::name);
    const char *a::attrNames[] = {
        "href",
        "title"
    };

    std::set<url> a::cached;
    std::set<url> a::uncached;

    a& a::href( const url& v ) {
        attrValues[hrefAttr] = v.string();
        if( v.protocol.empty() ) {
            std::set<url>::const_iterator found = cached.find(v);
            if( found == cached.end() ) {
                uncached.insert(v);
            }
        }
        return *this;
    }

    const char* body::name = "body";
    const detail::nodeEnd body::end(body::name,true);

    const char* caption::name = "caption";
    const detail::nodeEnd caption::end(caption::name,true);

    const char* div::name = "div";
    const detail::nodeEnd div::end(div::name,true);
    const char *div::attrNames[] = {
        "class"
    };

    const char *h::names[] = {
        "h1",
        "h2",
        "h3",
        "h4",
        "h5"
    };

    const char* form::name = "form";
    const detail::nodeEnd form::end(form::name);
    const char *form::attrNames[] = {
        "action",
        "class",
        "method"
    };

    const char* head::name = "head";
    const detail::nodeEnd head::end(head::name,true);

    const char* img::name = "img";
    const detail::nodeEnd img::end(img::name);
    const char *img::attrNames[] = {
        "class",
        "src"
    };

    const char* input::name = "input";
    const detail::nodeEnd input::end(input::name);
    const char *input::hidden = "hidden";
    const char *input::image = "image";
    const char *input::one = "1";
    const char *input::zero = "0";

    const char *input::attrNames[] = {
        "class",
        "name",
        "src",
        "type",
        "value"
    };

    const char* li::name = "li";
    const detail::nodeEnd li::end(li::name,true);

    const char *linebreak = "<br />\n";

    const char* p::name = "p";
    const detail::nodeEnd p::end(p::name,true);
    const char *p::attrNames[] = {
        "class"
    };

    const char* pre::name = "pre";
    const detail::nodeEnd pre::end(pre::name,true);
    const char *pre::attrNames[] = {
        "class"
    };

    const char* span::name = "span";
    const detail::nodeEnd span::end(span::name);
    const char *span::attrNames[] = {
        "class"
    };

    const char* table::name = "table";
    const detail::nodeEnd table::end(table::name,true);
    const char *table::attrNames[] = {
        "class"
    };


    const char* td::name = "td";
    const detail::nodeEnd td::end(td::name,true);
    const char *td::attrNames[] = {
        "class",
        "colspan"
    };

    const char* th::name = "th";
    const detail::nodeEnd th::end(th::name,true);
    const char *th::attrNames[] = {
        "class",
        "colspan"
    };

    const char* tr::name = "tr";
    const detail::nodeEnd tr::end(tr::name,true);
    const char *tr::attrNames[] = {
        "class"
    };

    const char* ul::name = "ul";
    const detail::nodeEnd ul::end(ul::name,true);

} // namespace html

const char* author::name = "author";
const detail::nodeEnd author::end(author::name);

const char* channel::name = "channel";
const detail::nodeEnd channel::end(channel::name);

const char* description::name = "description";
const detail::nodeEnd description::end(description::name);

const char* guid::name = "guid";
const detail::nodeEnd guid::end(guid::name);

const char* item::name = "item";
const detail::nodeEnd item::end(item::name);

const char* rsslink::name = "link";
const detail::nodeEnd rsslink::end(rsslink::name);

const char* pubDate::name = "pubDate";
const detail::nodeEnd pubDate::end(pubDate::name);

boost::posix_time::time_facet* pubDate::format() {
    // not creating a new object here leads to basic_string:S_create exceptions.
    return new boost::posix_time::time_facet("%a, %e %b %Y %H:%M:%S UT");
}


boost::posix_time::time_facet* pubDate::shortFormat() {
    // not creating a new object here leads to basic_string:S_create exceptions.
    return new boost::posix_time::time_facet("%a, %e %b %Y");
}


const char* rss::name = "rss";
const detail::nodeEnd rss::end(rss::name);
const char *rss::attrNames[] = {
    "version"
};


const char* title::name = "title";
const detail::nodeEnd title::end(title::name);

}
