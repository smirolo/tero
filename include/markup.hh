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

#ifndef guardmarkup
#define guardmarkup

#include <ostream>
#include <boost/date_time.hpp>
#include <boost/date_time/date_facet.hpp>
#include "webserve.hh"

/** Helper Markup tags on output stream

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/

namespace detail {

    class nodeEnd {
    public:
        bool newline;
        const char* name;

        explicit nodeEnd( const char* n, bool nl = false )
            : newline(nl), name(n) {}

        template<typename ch, typename tr>
        friend std::basic_ostream<ch, tr>&
        operator<<(std::basic_ostream<ch, tr>& ostr, const nodeEnd& v ) {
            ostr << "</" << v.name << '>';
            if( v.newline ) ostr << std::endl;
            return ostr;
        }
    };

    class attribute {
    public:
        bool valid;
        const char* name;
        std::string value;

        explicit attribute( const char* n )
            : valid(false), name(n) {}

        attribute( const char* n, const std::string& v )
            : valid(true), name(n), value(v) {}

        template<typename ch, typename tr>
        friend std::basic_ostream<ch, tr>&
        operator<<(std::basic_ostream<ch, tr>& ostr, const attribute& v ) {
            if( v.valid ) {
                ostr << ' ' << v.name << "=\"" << v.value << '\"';
            }
            return ostr;
        }
    };

    class markup {
    protected:
        bool newline;
        const char *name;

        const size_t attrArrayLength;
        const char **attrNames;
        std::string *attrValues;

   public:
        markup( const char *n,
            const char **attrNs,
            std::string *attrVs,
            const size_t attrNbs,
            bool nl = false ) : newline(nl), name(n),
                                attrArrayLength(attrNbs),
                                attrNames(attrNs), attrValues(attrVs)
        {}

        template<typename ch, typename tr>
        friend std::basic_ostream<ch, tr>&
        operator<<(std::basic_ostream<ch, tr>& ostr, const markup& v ) {
            ostr << "<" << v.name;
            if( (v.attrNames != NULL) & (v.attrValues != NULL) ) {
                for( size_t i = 0; i < v.attrArrayLength; ++i ) {
                    if( !v.attrValues[i].empty() ) {
                        ostr << ' ' << v.attrNames[i] << "=\"" << v.attrValues[i] << '\"';
                    }
                }
            }
            ostr << '>';
            /* \todo newline formatting should only appear when two tags
               are following each other with no text in between.
               The way to do it is with a decorator, not here. */
            if( v.newline ) ostr << std::endl;
            return ostr;
        }
    };

} // detail


std::string strip( const std::string& s );

std::string normalize( const std::string& s );

boost::posix_time::ptime from_mbox_string( const std::string& s );

std::string extractEmailAddress( const std::string& line );

std::string extractName( const std::string& );


namespace html {

    /** HTML a (href) markup
     */
    class a : public detail::markup {
    protected:
        enum attributes {
            hrefAttr,
            titleAttr
        };

        static const size_t attrLength = 2;

        static const char *attrNames[];
        std::string attrValues[attrLength];


    public:
        static const char* name;
        static const detail::nodeEnd end;
        static std::set<url> cached;
        static std::set<url> uncached;


        a() : markup(name,attrNames,attrValues,attrLength) {}

        a& href( const url& v );

        a& href( const std::string& v ) {
            return href(url(v));
        }

        a& title( const std::string& v ) {
            attrValues[titleAttr] = v;
            return *this;
        }
    };

    /** HTML body markup
     */
    class body : public detail::markup {
    public:
        static const char* name;
        static const detail::nodeEnd end;

        body() : markup(name,NULL,NULL,0,true) {}
    };


    /** HTML caption markup
     */
    class caption : public detail::markup {
    public:
        static const char* name;
        static const detail::nodeEnd end;

        caption() : markup(name,NULL,NULL,0,true) {}
    };

    /** HTML div markup
     */
    class div : public detail::markup {
    protected:
        enum attributes {
            classAttr,
        };

        static const size_t attrLength = 1;

        static const char *attrNames[];
        std::string attrValues[attrLength];

    public:
        static const char* name;
        static const detail::nodeEnd end;

        div() : markup(name,attrNames,attrValues,attrLength,true) {}

        div& classref( const char *v ) {
            attrValues[classAttr] = v;
            return *this;
        }
    };

    /** form markup
     */
    class form : public detail::markup {
    protected:
        enum attributes {
            actionAttr,
            classAttr,
            methodAttr
        };

        static const size_t attrLength = 3;

        static const char *attrNames[];
        std::string attrValues[attrLength];

    public:

        static const char* name;
        static const detail::nodeEnd end;

        form() : markup(name,attrNames,attrValues,attrLength,true) {}

        form& classref( const char *v ) {
            attrValues[classAttr] = v;
            return *this;
        }

        form& action( const url& u ) {
            attrValues[actionAttr] = u.string();
            return *this;
        }

        form& method( const char *v ) {
            attrValues[methodAttr] = v;
            return *this;
        }
    };

    /** HTML head markup
     */
    class head : public detail::markup {
    public:
        static const char* name;
        static const detail::nodeEnd end;

        head() : markup(name,NULL,NULL,0,true) {}
    };

    /** img markup
     */
    class img : public detail::markup {
    protected:
        enum attributes {
            classAttr,
            srcAttr,
        };

        static const size_t attrLength = 2;

        static const char *attrNames[];
        std::string attrValues[attrLength];

    public:

        static const char* name;
        static const detail::nodeEnd end;

        img() : markup(name,attrNames,attrValues,attrLength,true) {}

        img& classref( const char *v ) {
            attrValues[classAttr] = v;
            return *this;
        }

        img& src( const url& u ) {
            attrValues[srcAttr] = u.string();
            return *this;
        }

    };


    /** input markup
     */
    class input : public detail::markup {
    protected:
        enum attributes {
            classAttr,
            nameAttr,
            srcAttr,
            typeAttr,
            valueAttr
        };

        static const size_t attrLength = 5;

        static const char *attrNames[];
        std::string attrValues[attrLength];

        static const char* one;
        static const char* zero;
	
    public:

        static const char* name;
        static const detail::nodeEnd end;

        static const char* hidden;
        static const char* image;

        input() : markup(name,attrNames,attrValues,attrLength,true) {}

        input& classref( const char *v ) {
            attrValues[classAttr] = v;
            return *this;
        }

        input& nameref( const char *v ) {
            attrValues[nameAttr] = v;
            return *this;
        }

        input& src( const url& u ) {
            attrValues[srcAttr] = u.string();
            return *this;
        }

        input& type( const char *v ) {
            attrValues[typeAttr] = v;
            return *this;
        }

        input& value( bool b ) {
            attrValues[valueAttr] = b ? one : zero;
            return *this;
        }

        input& value( const std::string& s ) {
            attrValues[valueAttr] = s;
            return *this;
        }

        input& value( uint32_t v ) {
            std::stringstream s;
            s << v;
            attrValues[valueAttr] = s.str();
            return *this;
        }
    };


    /** HTML h{1,2,3,...} markup
     */
    class h : public detail::markup {
    protected:
        static const char* names[];

        int num;

    public:
        /* \todo assert than n < 5. */
        explicit h( int n ) : markup(names[n],NULL,NULL,0), num(n) {}

        detail::nodeEnd end() const {
            return detail::nodeEnd(names[num],true);
        }

    };

    /** HTML li markup
     */
    class li : public detail::markup {
    public:
        static const char* name;
        static const detail::nodeEnd end;

        li() : markup(name,NULL,NULL,0) {}
    };

    /** HTML linebreak markup
     */
    extern const char *linebreak;

    /** HTML p markup
     */
    class p : public detail::markup {
    protected:
        enum attributes {
            classAttr,
        };

        static const size_t attrLength = 1;

        static const char *attrNames[];
        std::string attrValues[attrLength];

    public:
        static const char* name;
        static const detail::nodeEnd end;

        p() : markup(name,attrNames,attrValues,attrLength,true) {}

        p& classref( const char *v ) {
            attrValues[classAttr] = v;
            return *this;
        }
    };

    /** pre markup
     */
    class pre : public detail::markup {
    protected:
        enum attributes {
            classAttr,
        };

        static const size_t attrLength = 1;

        static const char *attrNames[];
        std::string attrValues[attrLength];

    public:

        static const char* name;
        static const detail::nodeEnd end;

        pre() : markup(name,attrNames,attrValues,attrLength,true) {}

        pre& classref( const char *v ) {
            attrValues[classAttr] = v;
            return *this;
        }
    };

    /** span markup
     */
    class span : public detail::markup {
    protected:
        enum attributes {
            classAttr
        };

        static const size_t attrLength = 1;

        static const char *attrNames[];
        std::string attrValues[attrLength];

    public:

        static const char* name;
        static const detail::nodeEnd end;

        span() : markup(name,attrNames,attrValues,attrLength) {}

        span& classref( const char *v ) {
            attrValues[classAttr] = v;
            return *this;
        }
    };

    /** HTML table markup
     */
    class table : public detail::markup {
    public:
        enum attributes {
            classAttr,
        };

        static const size_t attrLength = 1;
        static const char *attrNames[];
        std::string attrValues[attrLength];

        static const char* name;
        static const detail::nodeEnd end;

        table() : markup(name,attrNames,attrValues,attrLength,true) {}

        table& classref( const char *v ) {
            attrValues[classAttr] = v;
            return *this;
        }

    };

    /** td markup
     */
    class td : public detail::markup {
    protected:
        enum attributes {
            classAttr,
            colspanAttr,
        };

        static const size_t attrLength = 2;

        static const char *attrNames[];
        std::string attrValues[attrLength];

    public:

        static const char* name;
        static const detail::nodeEnd end;

        td() : markup(name,attrNames,attrValues,attrLength,true) {}

        td& classref( const char *v ) {
            attrValues[classAttr] = v;
            return *this;
        }

        td& colspan( const char *v ) {
            attrValues[colspanAttr] = v;
            return *this;
        }

    };

    /** th markup
     */
    class th : public detail::markup {
    protected:
        enum attributes {
            classAttr,
            colspanAttr,
        };

        static const size_t attrLength = 2;

        static const char *attrNames[];
        std::string attrValues[attrLength];

    public:

        static const char* name;
        static const detail::nodeEnd end;

        th() : markup(name,attrNames,attrValues,attrLength,true) {}

        th& classref( const char *v ) {
            attrValues[classAttr] = v;
            return *this;
        }

        th& colspan( const char *v ) {
            attrValues[colspanAttr] = v;
            return *this;
        }

    };

    /** tr markup
     */
    class tr : public detail::markup {
    protected:
        enum attributes {
            classAttr
        };

        static const size_t attrLength = 1;

        static const char *attrNames[];
        std::string attrValues[attrLength];

    public:

        static const char* name;
        static const detail::nodeEnd end;

        tr() : markup(name,attrNames,attrValues,attrLength,true) {}

        tr& classref( const char *v ) {
            attrValues[classAttr] = v;
            return *this;
        }
    };

    /** HTML ul markup
     */
    class ul : public detail::markup {
    public:
        static const char* name;
        static const detail::nodeEnd end;

        ul() : markup(name,NULL,NULL,0,true) {}
    };


} // namespace html

/** RSS author markup

    This must be an e-mail address
 */
class author : public detail::markup {
public:
    static const char* name;
    static const detail::nodeEnd end;

    author() : markup(name,NULL,NULL,0) {}
};


/** channel markup
 */
class channel : public detail::markup {
public:
    static const char* name;
    static const detail::nodeEnd end;

    channel() : markup(name,NULL,NULL,0,true) {}
};


/** description markup
 */
class description : public detail::markup {
public:
    static const char* name;
    static const detail::nodeEnd end;

    description() : markup(name,NULL,NULL,0,true) {}
};


/** guid into a rss feed
 */
class guid : public detail::markup {
public:
    static const char* name;
    static const detail::nodeEnd end;

    guid() : markup(name,NULL,NULL,0) {}
};


/** item into a rss feed
 */
class item : public detail::markup {
public:
    static const char* name;
    static const detail::nodeEnd end;

    item() : markup(name,NULL,NULL,0,true) {}
};


/** link into a rss feed
 */
class rsslink : public detail::markup {
public:
    static const char* name;
    static const detail::nodeEnd end;

    rsslink() : markup(name,NULL,NULL,0) {}
};


/** a source code listing markup
 */
class code : public html::pre {
public:
    code() {}

    template<typename ch, typename tr>
    friend std::basic_ostream<ch, tr>&
    operator<<(std::basic_ostream<ch, tr>& ostr, const code& v ) {
        ostr << "<pre class=\"code\">" << std::endl;
        return ostr;
    }
};


/** RSS pubDate markup

    Following RFC 822 formatting.
 */
class pubDate : public detail::markup {
public:
    static const char* name;
    static const detail::nodeEnd end;
    boost::posix_time::ptime time;

    /** format string used for printing date_time
     */
    static const char *format;
    static const char *shortFormat;

    explicit pubDate( boost::posix_time::ptime t )
    : markup(name,NULL,NULL,0), time(t) {}

    template<typename ch, typename tr>
    friend std::basic_ostream<ch, tr>&
    operator<<( std::basic_ostream<ch, tr>& ostr, const pubDate& v ) {
        boost::posix_time::time_facet*
            facet(new boost::posix_time::time_facet(format));
        ostr.imbue(std::locale(ostr.getloc(), facet));
        ostr << static_cast<const detail::markup&>(v)
             << v.time
             << end;
        return ostr;
    }


};


/** rss markup
 */
class rss : public detail::markup {
protected:
    enum attributes {
        versionAttr,
    };

    static const size_t attrLength = 1;

    static const char *attrNames[];
    std::string attrValues[attrLength];

public:
    static const char* name;
    static const detail::nodeEnd end;

    rss() : markup(name,attrNames,attrValues,attrLength,true) {}

    rss& version( const char *v ) {
        attrValues[versionAttr] = v;
        return *this;
    }
};


/** title markup
 */
class title : public detail::markup {
public:
    static const char* name;
    static const detail::nodeEnd end;

    title() : markup(name,NULL,NULL,0) {}
};


template<typename ch, typename tr>
std::basic_ostream<ch, tr>&
mbox_string( std::basic_ostream<ch, tr>& ostr,
    const boost::posix_time::ptime& time )
{
    using namespace boost::gregorian;
    using namespace boost::posix_time;

    time_facet* facet(new time_facet(pubDate::format));
    ostr.imbue(std::locale(ostr.getloc(), facet));
    ostr << time;
    return ostr;
}


/** insert a comma separated list stored as a string into a set of strings.
 */
template<typename charIter, typename outIter>
outIter insertItems( charIter first, charIter last, outIter outs ) {
    charIter base = first;
    while( first != last ) {
        if( *first == ',' ) {
            std::string s = strip(std::string(base,first - base));
            if( !s.empty() ) {
                *outs++ = s;
            }
            base = first;
            ++base;
        }
        ++first;
    }
    std::string s = strip(std::string(base,first - base));
    if( !s.empty() ) {
        *outs++ = s;
    }
    return outs;
}


/** Write a pathname *base* / *leaf* as an html href link in an output stream *ostr*. 
 */
template<typename charT, typename traitsT>
std::basic_ostream<charT,traitsT>&
writelink( std::basic_ostream<charT,traitsT>& ostr,
    const boost::filesystem::path& base,
    const boost::filesystem::path& leaf,
    const std::string& ext = "" ) {
    if( !ext.empty() ) {
        ostr << html::a().href((base / leaf).string() + ext);
    } else {
        ostr << html::a().href((base / leaf).string());
    }
    ostr << leaf << html::a::end;
    return ostr;
}


#endif
