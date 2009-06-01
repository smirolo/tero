#ifndef guardLink
#define guardLink

#if 0

#include <ostream>
#include "xmltok.hh"

template<typename charT, typename traitsT = std::char_traits<charT> >
class basicLinkLight;

typedef basicLinkLight<char> linkLight;


template<typename charT, typename traitsT>
class basicLinkLight : virtual public std::basic_ostream<charT, traitsT> {
public:
	// Types (inherited from basic_ios (27.4.4)):
	typedef charT                     		char_type;
	typedef typename traitsT::int_type 		int_type;
	typedef typename traitsT::pos_type 	pos_type;
	typedef typename traitsT::off_type 		off_type;
	typedef traitsT                    		traits_type;
	
	// Non-standard types
	typedef basicLinkLight<charT, traitsT>	self;
	typedef std::basic_ostream<charT, traitsT> super;

	class xmlListener : public xmlTokListener {
	protected:
		super *ostr;
		
	public:
		explicit xmlListener( super& o ) : ostr(&o) {}
		
		void token( xmlToken token, const char *line, int first, int last, bool fragment ) {
			*ostr << '<' << xmlTokenTitles[token];
			if( fragment ) *ostr << " fragment=\"" << fragment << "\"";
			*ostr << " text=\"[" << first << "," << last << "]\">";
			std::copy(&line[first],&line[last],std::ostream_iterator<char>(*ostr));
			*ostr << "</" << xmlTokenTitles[token] << ">";
		}

	    void flush() {
		ostr->flush();
	    }
	};

protected:

	xmlListener listener;
 	xmlTokenizer tokenizer;

public:
    explicit basicLinkLight( super& ostr )
	: listener(ostr), tokenizer(listener) {}
    
    /** \brief  Character string insertion.
     */
    self& write( const char_type* s, std::streamsize n );
    
    /** \brief  Synchronizing the stream buffer.
	\return  *this
    */
    self& flush();
};


template<typename charT, typename traitsT>
typename basicLinkLight<charT,traitsT>::self& 
basicLinkLight<charT,traitsT>::write( const char_type* s, std::streamsize n ) {
	tokenizer.tokenize(s);
}


template<typename charT, typename traitsT>
typename basicLinkLight<charT,traitsT>::self&
basicLinkLight<charT,traitsT>::flush() {
    listener.flush();
    return *this;
}

#endif

#endif
