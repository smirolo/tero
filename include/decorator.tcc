// -*- C++ -*-
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
basicHighLight<tokenizerT,charT,traitsT>::~basicHighLight() {
    detach();
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
    super::next = &o;
    super::nextBuf = o.rdbuf(&buf);
}


template<typename tokenizerT, typename charT, typename traitsT>
void basicHighLight<tokenizerT,charT,traitsT>::detach() {
    if( super::next != NULL ) {
	sync();
	super::next->rdbuf(super::nextBuf);
	super::next = NULL;
    }
}


template<typename tokenizerT, typename charT, typename traitsT>
int basicHighLight<tokenizerT,charT,traitsT>::sync() { 
    if( super::next != NULL ) {
	assert( super::nextBuf != NULL );
	scan(); 
	return super::nextBuf->pubsync(); 
    }
    return 0;
}


template<typename tokenizerT, typename charT, typename traitsT>
void basicHighLight<tokenizerT,charT,traitsT>::scan() {
    int size = std::distance(buf.gptr(), buf.pptr());
#if 0
    super::nextBuf->sputn("tokenize:",9);
    super::nextBuf->sputn(buf.gptr(),size);
#endif
     tokenizer.tokenize(buf.gptr(),size);    
    buf.gbump(size);
}


template<typename charT, typename traitsT>
void basicHtmlEscaper<charT,traitsT>::token( xmlEscToken token, 
					     const char *line, 
					     int first, int last, 
					     bool fragment ) {
    switch( token ) {
    case escData:
	super::nextBuf->sputn(&line[first],last-first);
	break;
    case escLtEscape:
	super::nextBuf->sputn("&lt;",4);
	break;
    case escGtEscape:
	super::nextBuf->sputn("&gt;",4);
	break;
    }
}


template<typename charT, typename traitsT>   
void basicLinkLight<charT,traitsT>::token( xmlToken token, 
					   const char *line, 
					   int first, int last, 
					   bool fragment ) {
#if 0
    super::nextBuf->sputn(xmlTokenTitles[token],
			  strlen(xmlTokenTitles[token]));
#endif
    super::nextBuf->sputn(&line[first],last - first);
    switch( token ) {
    case xmlElementStart:
    case xmlElementEnd:
	/* Reset the state machine */ 
	state = linkStartState;
	break;
    case xmlName:
	if( strncmp(&line[first],"href",std::min(last - first,4)) == 0 ) {
	    /* Wait for attribute value */
	    state = linkWaitAttState;
	}
	break;
    case xmlAttValue:
	/* Categorize link */ 
	if( state == linkWaitAttState ) {
	    std::string name(&line[first + 1],last - first - 2);
	    url u(name);
	    if( u.absolute() ) {
		std::string absolute(" class=\"outside\"");
		super::nextBuf->sputn(absolute.c_str(),absolute.size());
	    } else {
		boost::filesystem::path f = context->abspath(name);
		if( f.empty() ) {
		    std::string absolute(" class=\"new\"");
		    super::nextBuf->sputn(absolute.c_str(),absolute.size());
		}
	    }
	}
	state = linkStartState;
	break;
    }	
}


template<typename charT, typename traitsT>   
void basicCppLight<charT,traitsT>::newline() {
    if( preprocessing ) {
	std::string endSpan("</span>");
	super::nextBuf->sputn(endSpan.c_str(),endSpan.size());
    }
    super::nextBuf->sputc('\n');
    if( virtualLineBreak ) {
	std::string staSpan("<span class=\"");
	super::nextBuf->sputn(staSpan.c_str(),staSpan.size());
	super::nextBuf->sputn(cppTokenTitles[cppPreprocessing],
			      strlen(cppTokenTitles[cppPreprocessing]));
	super::nextBuf->sputc('"');
	super::nextBuf->sputc('>');
    } else {
	preprocessing = false;
    }
}


template<typename charT, typename traitsT>   
void basicCppLight<charT,traitsT>::token( cppToken token, 
					  const char *line, 
					  int first, int last, 
					  bool fragment ) {
    std::string staSpan("<span class=\"");
    std::string endSpan("</span>");
    if( !preprocessing ) {
	super::nextBuf->sputn(staSpan.c_str(),staSpan.size());
	super::nextBuf->sputn(cppTokenTitles[token],
			      strlen(cppTokenTitles[token]));
	super::nextBuf->sputc('"');
	super::nextBuf->sputc('>');
	if( token == cppPreprocessing ) preprocessing = true;
    }
    if( token != cppComment ) {
	/* Special caracters are not replaced within comments
	   such that they can be used to mark up text as html. */
	for( ; first != last; ++first ) {
	    switch( line[first] ) {
	    case '<':
		super::nextBuf->sputn("&lt;",4);
		break;
	    case '>':
		super::nextBuf->sputn("&gt;",4);
		break;
	    default:
		super::nextBuf->sputc(line[first]);
	    }
	}
    } else {
	super::nextBuf->sputn(&line[first],last - first);
    }
    if( !preprocessing ) {
	super::nextBuf->sputn(endSpan.c_str(),endSpan.size());
    }
    virtualLineBreak = fragment;
}

