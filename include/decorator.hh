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

#ifndef guarddecorator
#define guarddecorator

#include <ostream>
#include "session.hh"
#include "cpptok.hh"
#include "xmltok.hh"

/* Decorators are used to highjack the underlying buffer of an ostream
   in order to format the text sent to the ostream.

   Decorators can be attached and detached on the fly, first-in last-out,
   to an ostream. In order to pass a reference to a chain of decorators,
   a decoratorChain class is defined (i.e. composite pattern).
*/

// forward declaration
template<typename charT, typename traitsT = std::char_traits<charT> >
class basicDecoratorChain;


/** \brief decorate the text outputed through a basic_ostream

	This is the base class for basic_ostream decorators. 
	Derived classes tokenize a stream of raw text and forwards 
	a stream of text decorated with tags to the underlying buffer.
 */
template<typename charT, typename traitsT = std::char_traits<charT> >
class basicDecorator : public std::basic_ostream<charT, traitsT> {
protected:
	friend class basicDecoratorChain<charT,traitsT>;

	typedef std::basic_ostream<charT, traitsT> super;

    std::basic_streambuf<charT, traitsT>* nextBuf;
    std::basic_ostream<charT, traitsT> *next;

	bool pre;

public:
    explicit basicDecorator( std::basic_streambuf<charT,traitsT> *sb, 
							 bool formated = false )
		: super(sb), next(NULL), nextBuf(sb), pre(formated) {
	}

	virtual ~basicDecorator() {}

	/** \brief Attach the decorator to a basic_ostream

		After the call returns, all text sent to the basic_ostream will appear
		decorated in the final output.
	 */
	virtual void attach(  std::basic_ostream<charT, traitsT>& o ) = 0;

	/** Detach the decorator from a basic_ostream

		After the call returns, the decorator will no longer preprocess 
		the text sent to the basic_ostream.
	 */
	virtual void detach() = 0;

	/** True when the decorator formats the underlying stream layout.

		When *formated* is True, an HTML processor will emit <pre> and </pre>
		tags around the decorated text. When *formated* is false, an HTML 
		processor will assume that spaces and carriage returns are not 
		formatters.
	 */
	bool formated() const { return pre; }

};

typedef basicDecorator<char> decorator;


/** \brief Composite for attaching multiple decorators as a single decorator.
 */
template<typename charT, typename traitsT>
class basicDecoratorChain : public basicDecorator<charT, traitsT> {
protected:
	typedef basicDecorator<charT, traitsT> super;

	super *first;
	super *last;

public:
	basicDecoratorChain() : super(NULL), first(NULL), last(NULL) {}

	virtual ~basicDecoratorChain();

	virtual void attach(  std::basic_ostream<charT, traitsT>& o );

	virtual void detach();

	void push_back( basicDecorator<charT, traitsT>& );

};

typedef basicDecoratorChain<char> decoratorChain;


/** \brief Base class to decorate streams of tokens.
 */
template<typename tokenizerT, typename charT, 
		 typename traitsT = std::char_traits<charT> >
class basicHighLight : public basicDecorator<charT, traitsT> {
protected:
	typedef basicDecorator<charT, traitsT> super;
	using basicDecorator<charT, traitsT>::next;
	using basicDecorator<charT, traitsT>::nextBuf;
	
private:
    class buffer : public std::basic_stringbuf<charT, traitsT> {
    public:
		using		std::basic_stringbuf<charT, traitsT>::gbump;
		using		std::basic_stringbuf<charT, traitsT>::gptr;
		using		std::basic_stringbuf<charT, traitsT>::pptr;

		explicit buffer( basicHighLight& d ) 
			: std::basic_stringbuf<charT, traitsT>(),
			  decorator(d) {}
		
		int sync() { return decorator.sync(); }
		
		basicHighLight& decorator;
    };
    
	basicHighLight( const basicHighLight& ); // not defined
    basicHighLight& operator=(const basicHighLight& ); // not defined
   
    buffer buf;

protected:
	tokenizerT tokenizer;

	void scan();

public:
	explicit basicHighLight( bool formated );

    explicit basicHighLight( std::basic_ostream<charT,traitsT>& o, 
							 bool formated = false );

	virtual ~basicHighLight() {
		detach();
	}

	virtual void attach( std::ostream& o );

	virtual void detach();

    int	sync();

};


/** \brief Decorate links with load, create and external. 
 */
template<typename charT, typename traitsT = std::char_traits<charT> >
class basicLinkLight : public basicHighLight<xmlTokenizer, charT, traitsT>,
					   public xmlTokListener {
protected:
    typedef basicHighLight<xmlTokenizer, charT, traitsT> super;

	enum {
		linkStartState,
		linkWaitAttState
	} state;

	session* context;
    
public:
	explicit basicLinkLight( session& s ) 
		: super(false), context(&s) { 
		super::tokenizer.attach(*this); 
	}

    explicit basicLinkLight(  session& s, std::basic_ostream<charT,traitsT>& o )
		: super(o,false), context(&s) { super::tokenizer.attach(*this); }

	void newline() {
		super::nextBuf->sputc('\n');
	}

	void token( xmlToken token, const char *line, 
				int first, int last, bool fragment ) {
		super::nextBuf->sputn(&line[first],last - first);
		switch( token ) {
		case xmlStartDecl:
		case xmlEndDecl:
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
					boost::filesystem::path f = context->findFile(name);
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

};

typedef basicLinkLight<char> linkLight;


/** \brief Decorate C++ code with token tags.
 */
template<typename charT, typename traitsT = std::char_traits<charT> >
class basicCppLight : public basicHighLight<cppTokenizer,charT,traitsT>,
                       public cppTokListener {
protected:
	bool preprocessing;
	bool virtualLineBreak;
    typedef basicHighLight<cppTokenizer,charT, traitsT>  super;

public:
	basicCppLight() 
		: super(true), preprocessing(false), virtualLineBreak(false) { 
		super::tokenizer.attach(*this); 
	}

    explicit basicCppLight( std::basic_ostream<charT,traitsT>& o )
		: super(o,true), preprocessing(false), virtualLineBreak(false) { 
		super::tokenizer.attach(*this); 
	}

	void newline() {
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

	void token( cppToken token, const char *line, int first, int last, bool fragment ) {
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
		if( !preprocessing ) {
			super::nextBuf->sputn(endSpan.c_str(),endSpan.size());
		}
		virtualLineBreak = fragment;
	}
};

typedef basicCppLight<char> cppLight;

#include "decorator.tcc"

#endif
