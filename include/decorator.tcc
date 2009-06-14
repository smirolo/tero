/* Copyright (c) 2009, Sebastien Mirolo
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of codespin nor the
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

// -*- C++ -*-

template<typename charT, typename traitsT>
basicDecoratorChain<charT,traitsT>::~basicDecoratorChain() {
	detach();
}


template<typename charT, typename traitsT>
void basicDecoratorChain<charT,traitsT>::attach( 
	     std::basic_ostream<charT, traitsT>& o )
{	 
	if( last ) {
		assert( first != NULL );
		last->next = &o;
		last->nextBuf = o.rdbuf(first->rdbuf());
	}
}


template<typename charT, typename traitsT>
void basicDecoratorChain<charT,traitsT>::detach()
{
	if( last ) {
		last->detach();
	}
}


template<typename charT, typename traitsT>
void basicDecoratorChain<charT,traitsT>::push_back( 
         basicDecorator<charT, traitsT>& d ) 
{
	if( first ) {
		d.attach(*first);
	} else {
	    last = &d;
    }
	first = &d;
	super::pre |= d.formated();
}


template<typename tokenizerT, typename charT, typename traitsT>
basicHighLight<tokenizerT,charT,traitsT>::basicHighLight( bool formated )
    : super(&buf,formated),
      buf(*this)
{
}


template<typename tokenizerT, typename charT, typename traitsT>
basicHighLight<tokenizerT,charT,traitsT>::basicHighLight( 
    std::basic_ostream<charT,traitsT>& o, bool formated ) 
    : super(&buf,formated),
      buf(*this)
{
	attach(o);
}


template<typename tokenizerT, typename charT, typename traitsT>
void basicHighLight<tokenizerT,charT,traitsT>::attach( std::ostream& o ) { 
	/* !!! This method relies on a correct usage pattern as it will not detach 
	   a previously attached stream. It does not call detach as a decorator 
	   chain could have setup the decorator as part of a sequence. */
	next = &o;
	nextBuf = o.rdbuf(&buf);
}


template<typename tokenizerT, typename charT, typename traitsT>
void basicHighLight<tokenizerT,charT,traitsT>::detach() {
	if( next != NULL ) {
		sync();
		next->rdbuf(nextBuf);
		next = NULL;
	}
}


template<typename tokenizerT, typename charT, typename traitsT>
int	basicHighLight<tokenizerT,charT,traitsT>::sync() { 
	if( next != NULL ) {
		assert( nextBuf != NULL );
		scan(); return nextBuf->pubsync(); 
	}
	return 0;
}


template<typename tokenizerT, typename charT, typename traitsT>
void basicHighLight<tokenizerT,charT,traitsT>::scan() {
    int size = std::distance(buf.gptr(), buf.pptr());
    tokenizer.tokenize(buf.gptr(),size);    
    buf.gbump(size);
}
