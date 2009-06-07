// -*- C++ -*-

template<typename charT, typename traitsT>
basicDecoratorChain<charT,traitsT>::~basicDecoratorChain() {
	detach();
}


template<typename charT, typename traitsT>
void basicDecoratorChain<charT,traitsT>::attach( std::basic_ostream<charT, traitsT>& o )
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
void basicDecoratorChain<charT,traitsT>::push_back( basicDecorator<charT, traitsT>& d ) 
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
basicHighLight<tokenizerT,charT,traitsT>::basicHighLight( std::basic_ostream<charT,traitsT>& o, bool formated ) 
    : super(&buf,formated),
      buf(*this)
{
	attach(o);
}


template<typename tokenizerT, typename charT, typename traitsT>
void basicHighLight<tokenizerT,charT,traitsT>::attach( std::ostream& o ) { 
	/* !!! This method relies on a correct usage pattern as it will not detach 
	   a previously attached stream. It does not call detach as a decorator chain 
	   could have setup the decorator as part of a sequence. */
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
