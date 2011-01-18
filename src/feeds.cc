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

#include "feeds.hh"

feedIndex::indexSet feedIndex::indices;

const feedIndex::difference_type feedIndex::maxLength = ~((int)1 << ((sizeof(int) << 3) - 1));

feedIndex::iterator feedIndex::first;
feedIndex::iterator feedIndex::last;

feedIndex feedIndex::instance("",0,feedIndex::maxLength);

void feedIndex::filters( const post& p ) {
    /* create one shortPost per post tag. */
    post clean = p;
    clean.normalize();
    if( p.tags.empty() ) {
	indices.push_back(clean);
    } else {
	for( post::tagSet::const_iterator t = p.tags.begin();
	     t != p.tags.end(); ++t ) {
	    indices.push_back(post(clean,*t));
	}
    }
    first = indices.begin();
    last = indices.end();
}


void feedIndex::provide() {
    if( std::distance(first,last) >= base ) {
	std::advance(first,base);
    }
    indexSet::iterator second = second;
    if( std::distance(second,last) >= length ) {
	std::advance(second,length);
    } else {
	second = last;
    }
    last = second;
}

