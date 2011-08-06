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

/**
   Detail implementation for decorators.

   Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/

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
    tokenizer.tokenize(buf.gptr(),size);    
    buf.gbump(size);
}


template<typename charT, typename traitsT>
void basicHtmlEscaper<charT,traitsT>::token( xmlEscToken token, 
					     const char *line, 
					     int first, int last, 
					     bool fragment ) {
    switch( token ) {
    case escErr:
    case escData:
	super::nextBuf->sputn(&line[first],last-first);
	break;
    case escAmpEscape:
	super::nextBuf->sputn("&amp;",5);
	break;
    case escLtEscape:
	super::nextBuf->sputn("&lt;",4);
	break;
    case escGtEscape:
	super::nextBuf->sputn("&gt;",4);
	break;
    case escQuotEscape:
	super::nextBuf->sputn("&quot;",6);
	break;
    }
}

template<typename charT, typename traitsT>
typename basicLinkLight<charT,traitsT>::linkSet 
basicLinkLight<charT,traitsT>::allLinks;

template<typename charT, typename traitsT>
typename basicLinkLight<charT,traitsT>::linkSet 
basicLinkLight<charT,traitsT>::currs;

template<typename charT, typename traitsT>
typename basicLinkLight<charT,traitsT>::linkSet 
basicLinkLight<charT,traitsT>::nexts;


template<typename charT, typename traitsT>
typename basicLinkLight<charT,traitsT>::linkClass 
basicLinkLight<charT,traitsT>::add( const url& u ) {
#if 0
    std::cerr << "consider " << u;
#endif
    if( u.host.empty() || u.host == domainName.value(*context).host ) {
	url f = context->asUrl(context->abspath(u));
	const fetchEntry* e = dispatchDoc::instance()->select("view",u.string());
	if( e->behavior != always ) {
	    if( allLinks.find(f) == allLinks.end() 
		&& currs.find(f) == currs.end() ) {	
		/* we have never seen that vertex before (i.e. white)
		   so let's add it to the list of successors to process. */
#if 0
		std::cerr << ", add " << f;
#endif
		nexts.insert(f);
	    }
#if 0
	    std::cerr << std::endl;
#endif
	    return localFileExists;
	}
#if 0
	std::cerr << std::endl;
#endif
	return localLinkGenerated;		
    }
#if 0
    std::cerr << std::endl;
#endif
    return remoteLink; 
}


template<typename charT, typename traitsT>   
bool basicLinkLight<charT,traitsT>::decorate( const url& u )
{
    super::nextBuf->sputc('"');
    super::nextBuf->sputn(u.string().c_str(),u.string().size());
    super::nextBuf->sputc('"');	

    switch( add(u) ) {
    case localFileExists:
		break;
    case localLinkGenerated: {
		std::string absolute(" class=\"new\"");
		super::nextBuf->sputn(absolute.c_str(),absolute.size());
    } break;
    case remoteLink: {
		std::string absolute(" class=\"outside\"");
		super::nextBuf->sputn(absolute.c_str(),absolute.size());
    } break;
    }

    return false;
}


template<typename charT, typename traitsT>   
void basicLinkLight<charT,traitsT>::token( xmlToken token, 
					   const char *line, 
					   int first, int last, 
					   bool fragment ) {
    bool needPut = true;
    
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
	    needPut = decorate(url(name));
	}
	state = linkStartState;
	break;
    default:
	/* Nothing to do except prevent gcc from complaining. */
	break;
    }
    if( needPut ) super::nextBuf->sputn(&line[first],last - first);
}


template<typename charT, typename traitsT>   
bool absUrlDecoratorBase<charT,traitsT>::decorate( const url& u )
{
    url a = super::context->asAbsUrl(u,base);
    super::nextBuf->sputc('"');
    super::nextBuf->sputn(a.string().c_str(),a.string().size());
    super::nextBuf->sputc('"');	
    return false;
}


template<typename charT, typename traitsT>   
bool cachedUrlBase<charT,traitsT>::decorate( const url& u )
{
    super::nextBuf->sputc('"');
    switch( super::add(u) ) {
    case super::localFileExists: {
	url cached(super::context->cacheName(u));
	super::nextBuf->sputn(cached.string().c_str(),cached.string().size());
    } break;
    default:
	super::nextBuf->sputn(u.string().c_str(),u.string().size());
	break;
    }    
    super::nextBuf->sputc('"');	

    return false;
}


template<typename charT, typename traitsT>
void basicHrefLight<charT,traitsT>::token( hrefToken token, const char *line, 
					   int first, int last, bool fragment ) 
{
    switch( token ) {
    case hrefFilename: {
	std::string href("<a href=\"");
	std::string hrefEnd("</a>");
	super::nextBuf->sputn(href.c_str(),href.size());
	super::nextBuf->sputn(&line[first],last - first);
	super::nextBuf->sputc('"');
	super::nextBuf->sputc('>');	
	super::nextBuf->sputn(&line[first],last - first);
	super::nextBuf->sputn(hrefEnd.c_str(),hrefEnd.size());
    } break;
    default:
	super::nextBuf->sputn(&line[first],last - first);	
	break;
    }
}
    



template<typename charT, typename traitsT>   
void basicCppLight<charT,traitsT>::newline(const char *line, 
					   int first, int last )
{
    if( preprocessing ) {
	std::string endSpan("</span>");
	super::nextBuf->sputn(endSpan.c_str(),endSpan.size());
    }
    super::nextBuf->sputc('\n');
    if( preprocessing & virtualLineBreak ) {
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

