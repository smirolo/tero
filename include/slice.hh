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

#ifndef guardslice
#define guardslice

#include <cassert>
#include <ostream>

/**
   Slice of text in memory.

   Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/

namespace tero {

template<typename vT>
class slice {
public:
    typedef vT* iterator;
    typedef const vT* const_iterator;

protected:
    vT* first;
    vT* last;

public:
    slice() : first(NULL), last(NULL) {}

    slice( vT* f, vT* l )
        : first(f), last(l) {}

    iterator begin() { return first; }
    const_iterator begin() const { return first; }
    iterator end() { return last; }
    const_iterator end() const { return last; }

    size_t size() const {
        return last - first;
    }

    slice& operator+=( const slice& s ) {
        if( first == last ) {
            first = s.first;
            last = s.last;
        } else {
            assert( last == s.first );
            last = s.last;
        }
        return *this;
    }
};


/** Shrink a slice of characters such as to discard leading
    and trailing whitespaces.
 */
template<typename charT>
const slice<charT> strip( const slice<charT>& s ) {
    typename slice<charT>::const_iterator p = s.begin();
    while( isspace(*p) & (p != s.end()) ) ++p;
    typename slice<charT>::const_iterator q = s.end();
    do {
        --q;
    } while( isspace(*q) & (q != p) );
    return slice<charT>(p,q + 1);
}


template<typename ch, typename tr, typename vT>
inline std::basic_ostream<ch, tr>&
operator<<( std::basic_ostream<ch, tr>& ostr, const slice<vT>& v ) {
    ostr << "[" << static_cast<const void*>(v.begin())
         << ',' << static_cast<const void*>(v.end()) << "[";
    return ostr;
}


template<typename ch, typename tr>
inline std::basic_ostream<ch, tr>&
operator<<( std::basic_ostream<ch, tr>& ostr, const slice<char>& v ) {
    ostr.write(v.begin(),v.size());
    return ostr;
}


template<typename ch, typename tr>
inline std::basic_ostream<ch, tr>&
operator<<( std::basic_ostream<ch, tr>& ostr, const slice<const char>& v ) {
    ostr.write(v.begin(),v.size());
    return ostr;
}

}

#endif
